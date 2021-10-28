#include "View.h"
#include "Context.h"
#include "Material.h"
#include "Mesh.h"
#include "Pipeline.h"
#include "RenderPass.h"
#include "Scene.h"
#include "features/Renderer.h"
#include <ars/runtime/core/Log.h>

namespace ars::render::vk {
namespace {
VkExtent2D translate(const Extent2D &size) {
    return {size.width, size.height};
}
} // namespace

void View::render() {
    auto ctx = context();
    _rt_manager->update(translate(_size));

    ctx->queue()->submit_once(
        [&](CommandBuffer *cmd) { _renderer->render(cmd); });

    update_color_tex_adapter();
}

ITexture *View::get_color_texture() {
    return _color_tex_adapter.get();
}

View::View(Scene *scene, const Extent2D &size) : _scene(scene), _size(size) {
    _rt_manager = std::make_unique<RenderTargetManager>(scene->context());
    _rt_manager->update(translate(size));

    alloc_render_targets();

    _renderer = std::make_unique<Renderer>(this);
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

void View::update_color_tex_adapter() {
    auto color_rt = render_target(NamedRT_FinalColor);
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
        tex.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        break;
    }
    case NamedRT_FinalColor: {
        auto &tex = info.texture = translate(color_tex_info());
        tex.usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        break;
    }
    case NamedRT_Count:
        break;
    }

    return info;
}

std::unique_ptr<RenderPass> View::create_single_pass_render_pass(
    NamedRT *colors, uint32_t color_count, NamedRT depth_stencil) {
    auto attach_count = color_count;
    bool has_depth_stencil = depth_stencil != NamedRT_Count;
    if (has_depth_stencil) {
        attach_count += 1;
    }

    std::vector<VkAttachmentDescription> attachments{};
    attachments.resize(attach_count);
    auto &depth_attach = attachments.back();

    auto init_attach_desc = [&](VkAttachmentDescription &desc, NamedRT rt) {
        auto info = rt_info(rt);
        desc.format = info.texture.format;
        desc.samples = info.texture.samples;
        desc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        desc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        desc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        desc.finalLayout = VK_IMAGE_LAYOUT_GENERAL;
    };

    for (int i = 0; i < color_count; i++) {
        init_attach_desc(attachments[i], colors[i]);
    }
    if (has_depth_stencil) {
        init_attach_desc(depth_attach, depth_stencil);
    }

    std::vector<VkAttachmentReference> color_refs{};
    color_refs.resize(color_count);
    for (int i = 0; i < color_count; i++) {
        auto &ref = color_refs[i];
        ref.attachment = i;
        ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    }

    VkAttachmentReference depth_ref{};
    depth_ref.attachment = color_count;
    depth_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = color_count;
    subpass.pColorAttachments = color_refs.data();
    if (has_depth_stencil) {
        subpass.pDepthStencilAttachment = &depth_ref;
    }

    VkRenderPassCreateInfo info{VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
    info.attachmentCount = static_cast<uint32_t>(std::size(attachments));
    info.pAttachments = attachments.data();
    info.subpassCount = 1;
    info.pSubpasses = &subpass;

    return std::make_unique<RenderPass>(context(), info);
}

} // namespace ars::render::vk