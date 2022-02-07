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
#include <imgui/imgui.h>

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

    RenderGraph rg(this);

    auto final_rt = _renderer->render(rg);
    _overlay_renderer->render(rg, final_rt);

    rg.compile();
    rg.execute();

    update_color_tex_adapter(final_rt);
    flip_history_buffer();
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
    // TODO accept other rts
    if (rt != NamedRT_PostProcessing0 && rt != NamedRT_PostProcessing1) {
        ARS_LOG_WARN("Final rt color is neither NamedRT_PostProcessing0 nor "
                     "NamedRT_PostProcessing1, the TextureInfo of color "
                     "texture may be incorrect");
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
    info.mipmap_mode = MipmapMode::Linear;
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
    case NamedRT_LinearColor:
    case NamedRT_LinearColorHistory:
    case NamedRT_PostProcessing0:
    case NamedRT_PostProcessing1: {
        auto &tex = info.texture = translate(color_tex_info());
        tex.usage |=
            VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
        break;
    }
    case NamedRT_HiZBuffer: {
        auto &tex = info.texture = TextureCreateInfo::sampled_2d(
            VK_FORMAT_R32_SFLOAT, 1, 1, MAX_MIP_LEVELS);
        tex.usage |= VK_IMAGE_USAGE_STORAGE_BIT;
        tex.min_filter = VK_FILTER_NEAREST;
        tex.mag_filter = VK_FILTER_NEAREST;
        tex.mipmap_mode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
        tex.address_mode_u = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        tex.address_mode_v = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        tex.address_mode_w = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        break;
    }
    case NamedRT_ReflectionHistory:
    case NamedRT_Reflection: {
        auto &tex = info.texture = TextureCreateInfo::sampled_2d(
            VK_FORMAT_B10G11R11_UFLOAT_PACK32, 1, 1, 1);
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

void View::debug_gui() {
    ImGui::Begin("Vulkan Debug");
    auto width = ImGui::GetContentRegionAvailWidth();
    auto draw_rt = [&](const char *name, NamedRT rt) {
        ImGui::Text("%s", name);
        auto tex = render_target(rt);
        auto extent = tex->info().extent;
        float height = static_cast<float>(extent.height) /
                       static_cast<float>(extent.width) * width;
        ImGui::Image(render_target(rt).get(), {width, height});
    };

    draw_rt("GBuffer 0", NamedRT_GBuffer0);
    draw_rt("GBuffer 1", NamedRT_GBuffer1);
    draw_rt("GBuffer 2", NamedRT_GBuffer2);
    draw_rt("GBuffer 3", NamedRT_GBuffer3);
    draw_rt("Reflection", NamedRT_Reflection);
    ImGui::End();
}

void View::flip_history_buffer() {
    std::swap(_rt_ids[NamedRT_Reflection], _rt_ids[NamedRT_ReflectionHistory]);
    std::swap(_rt_ids[NamedRT_LinearColor],
              _rt_ids[NamedRT_LinearColorHistory]);
}
} // namespace ars::render::vk