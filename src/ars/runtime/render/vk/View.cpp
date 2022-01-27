#include "View.h"
#include "Context.h"
#include "Effect.h"
#include "Environment.h"
#include "Material.h"
#include "Mesh.h"
#include "Pipeline.h"
#include "Profiler.h"
#include "Scene.h"
#include "features/Drawer.h"
#include "features/OverlayRenderer.h"
#include "features/Renderer.h"
#include <ars/runtime/core/Log.h>

namespace ars::render::vk {
namespace {
VkExtent2D translate(const Extent2D &size) {
    return {size.width, size.height};
}
} // namespace

void View::render() {
    ARS_PROFILER_SAMPLE("Render View", 0xFFAA6611);
    auto ctx = context();
    _rt_manager->update(translate(_size));

    auto final_rt = ctx->queue()->submit_once([&](CommandBuffer *cmd) {
        ARS_PROFILER_SAMPLE_VK(cmd, "Render Scene", 0xFF772183);
        return _renderer->render(cmd);
    });

    if (_overlay_renderer->need_render()) {
        ctx->queue()->submit_once([&](CommandBuffer *cmd) {
            ARS_PROFILER_SAMPLE_VK(cmd, "Render Overlay", 0xFF152439);
            _overlay_renderer->render(cmd, final_rt);
        });
    }

    update_color_tex_adapter(final_rt);
}

ITexture *View::get_color_texture() {
    return _color_tex_adapter.get();
}

View::View(Scene *scene, const Extent2D &size) : _scene(scene), _size(size) {
    _rt_manager = std::make_unique<RenderTargetManager>(scene->context());
    _rt_manager->update(translate(size));

    alloc_render_targets();

    _renderer = std::make_unique<Renderer>(this);
    _overlay_renderer = std::make_unique<OverlayRenderer>(this);
    _drawer = std::make_unique<Drawer>(this);
    _effect = std::make_unique<Effect>();
}

math::XformTRS<float> View::xform() {
    return _xform;
}

void View::set_xform(const math::XformTRS<float> &xform) {
    _xform = xform;
}

void View::set_camera(const CameraData &camera) {
    _camera = camera;
}

CameraData View::camera() {
    return _camera;
}

Extent2D View::size() {
    return _size;
}

void View::set_size(const Extent2D &size) {
    _size = size;
}

void View::update_color_tex_adapter(NamedRT rt) {
    if (rt != NamedRT_FinalColor0 && rt != NamedRT_FinalColor1) {
        ARS_LOG_ERROR("Final result from renderer must be either FinalColor0 "
                      "or FinalColor1");
    }
    auto color_rt = render_target(rt);
    auto rt_info = color_tex_info();
    if (_color_tex_adapter == nullptr) {
        _color_tex_adapter =
            std::make_unique<TextureAdapter>(rt_info, color_rt);
    } else {
        *_color_tex_adapter = TextureAdapter(rt_info, color_rt);
    }
}

TextureInfo View::color_tex_info() const {
    TextureInfo info{};
    info.format = Format::B10G11R11_UFLOAT_PACK32;
    info.width = _size.width;
    info.height = _size.height;
    info.mip_levels = 1;
    info.mipmap_mode = MipmapMode::Nearest;
    return info;
}

void View::alloc_render_targets() {
    auto alloc_named = [&](NamedRT name) {
        _rt_ids[name] = _rt_manager->alloc(rt_info(name));
    };
    for (int i = 0; i < NamedRT_Count; i++) {
        alloc_named(static_cast<NamedRT>(i));
    }
}

View::~View() {
    auto ctx = context();
    ctx->queue()->flush();

    _renderer.reset();
}

Context *View::context() const {
    return _scene->context();
}

Handle<Texture> View::render_target(NamedRT name) const {
    return _rt_manager->get(_rt_ids[name]);
}

RenderTargetManager *View::rt_manager() const {
    return _rt_manager.get();
}

RenderTargetId View::rt_id(NamedRT name) const {
    return _rt_ids[name];
}

Scene *View::vk_scene() const {
    return _scene;
}

RenderTargetInfo View::rt_info(NamedRT name) const {
    RenderTargetInfo info{};

    auto color_attachment = [&](VkFormat format) {
        auto tex = TextureCreateInfo::sampled_2d(format, 1, 1, 1);
        tex.usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        return tex;
    };

    switch (name) {
    case NamedRT_GBuffer0: {
        info.texture = color_attachment(VK_FORMAT_R8G8B8A8_SRGB);
        break;
    }
    case NamedRT_GBuffer1: {
        info.texture = color_attachment(VK_FORMAT_A2R10G10B10_UNORM_PACK32);
        break;
    }
    case NamedRT_GBuffer2: {
        info.texture = color_attachment(VK_FORMAT_R8G8B8A8_UNORM);
        break;
    }
    case NamedRT_GBuffer3: {
        info.texture = color_attachment(VK_FORMAT_B10G11R11_UFLOAT_PACK32);
        break;
    }
    case NamedRT_Depth: {
        auto &tex = info.texture =
            TextureCreateInfo::sampled_2d(VK_FORMAT_D32_SFLOAT, 1, 1, 1);
        tex.aspect_mask = VK_IMAGE_ASPECT_DEPTH_BIT;
        tex.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT |
                    VK_IMAGE_USAGE_SAMPLED_BIT;
        break;
    }
    case NamedRT_FinalColor0:
    case NamedRT_FinalColor1: {
        auto &tex = info.texture = translate(color_tex_info());
        tex.usage |=
            VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
        break;
    }
    case NamedRT_HiZBuffer: {
        auto &tex = info.texture =
            TextureCreateInfo::sampled_2d(VK_FORMAT_R32G32_SFLOAT, 1, 1, 1);
        tex.usage |= VK_IMAGE_USAGE_STORAGE_BIT;
        break;
    }
    case NamedRT_Count:
        break;
    }

    return info;
}

std::unique_ptr<RenderPass> View::create_single_pass_render_pass(
    NamedRT *colors, uint32_t color_count, NamedRT depth_stencil) const {
    std::vector<RenderPassAttachmentInfo> attachments{};
    attachments.resize(color_count + 1);

    auto init_attach_info = [&](RenderPassAttachmentInfo &attach, NamedRT rt) {
        auto info = rt_info(rt);
        attach.format = info.texture.format;
        attach.samples = info.texture.samples;
    };

    for (int i = 0; i < color_count; i++) {
        init_attach_info(attachments[i], colors[i]);
    }

    bool has_depth_stencil = depth_stencil != NamedRT_Count;
    if (has_depth_stencil) {
        init_attach_info(attachments.back(), depth_stencil);
    }

    return RenderPass::create_with_single_pass(
        context(),
        color_count,
        attachments.data(),
        has_depth_stencil ? &attachments.back() : nullptr);
}

std::vector<uint64_t>
View::query_selection(uint32_t x, uint32_t y, uint32_t width, uint32_t height) {
    return _renderer->query_selection(x, y, width, height);
}

IOverlay *View::overlay() {
    return _overlay_renderer.get();
}

Drawer *View::drawer() const {
    return _drawer.get();
}

OverlayRenderer *View::vk_overlay() const {
    return _overlay_renderer.get();
}

std::shared_ptr<IEnvironment> View::environment() {
    return environment_vk();
}

void View::set_environment(const std::shared_ptr<IEnvironment> &environment) {
    _environment = std::dynamic_pointer_cast<Environment>(environment);
}

std::shared_ptr<Environment> View::environment_vk() {
    if (_environment == nullptr) {
        set_environment(_scene->context()->create_environment());
    }
    return _environment;
}

IEffect *View::effect() {
    return _effect.get();
}
} // namespace ars::render::vk