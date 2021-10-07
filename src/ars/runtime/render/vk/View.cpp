#include "View.h"
#include "Context.h"
#include "Scene.h"
#include <ars/runtime/core/Log.h>

namespace ars::render::vk {
namespace {
VkExtent2D translate(const Extent2D &size) {
    return {size.width, size.height};
}
} // namespace

void View::render() {
    auto context = _scene->context();
    _rt_manager->update(translate(_size));
    auto color_rt = _rt_manager->get(_color_rt_id);
    VkExtent2D extent{
        color_rt->info().extent.width,
        color_rt->info().extent.height,
    };

    VkFramebufferCreateInfo fb_info{VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
    fb_info.width = extent.width;
    fb_info.height = extent.height;
    fb_info.renderPass = _render_pass;
    fb_info.layers = 1;

    VkImageView attachments[1] = {color_rt->image_view()};
    fb_info.attachmentCount = static_cast<uint32_t>(std::size(attachments));
    fb_info.pAttachments = attachments;

    auto fb = context->create_tmp_framebuffer(&fb_info);
    context->queue()->submit_once([&](CommandBuffer *cmd) {
        VkClearValue clear_value{};
        auto &clear_color = clear_value.color.float32;
        clear_color[0] = 1.0f;
        clear_color[1] = 0.0f;
        clear_color[2] = 0.0f;
        clear_color[3] = 1.0f;

        VkRenderPassBeginInfo rp_begin{
            VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
        rp_begin.renderPass = _render_pass;
        rp_begin.framebuffer = fb;
        rp_begin.clearValueCount = 1;
        rp_begin.pClearValues = &clear_value;
        rp_begin.renderArea = {{0, 0}, extent};

        cmd->BeginRenderPass(&rp_begin, VK_SUBPASS_CONTENTS_INLINE);
        cmd->EndRenderPass();

        color_rt->assure_layout(VK_IMAGE_LAYOUT_GENERAL);
    });

    update_color_tex_adapter();
}

ITexture *View::get_color_texture() {
    return _color_tex_adapter.get();
}

View::View(Scene *scene, const Extent2D &size) : _scene(scene), _size(size) {
    _rt_manager = std::make_unique<RenderTargetManager>(scene->context());
    _rt_manager->update(translate(size));

    alloc_render_targets();
    init_render_pass();
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
    auto color_rt = _rt_manager->get(_color_rt_id);
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
    info.width = _size.width;
    info.height = _size.height;
    info.mip_levels = 1;
    info.mipmap_mode = MipmapMode::Nearest;
    return info;
}

void View::alloc_render_targets() {
    RenderTargetInfo color_info{};
    auto &tex = color_info.texture = translate(color_tex_info());
    tex.usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    color_info.multi_buffer_count = 1;

    _color_rt_id = _rt_manager->alloc(color_info);
}

View::~View() {
    auto device = _scene->context()->device();
    if (_render_pass != VK_NULL_HANDLE) {
        device->Destroy(_render_pass);
    }
}

void View::init_render_pass() {
    auto color_info = translate(color_tex_info());

    VkAttachmentDescription color_attachment{};
    color_attachment.format = color_info.format;
    color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    color_attachment.finalLayout = VK_IMAGE_LAYOUT_GENERAL;

    VkAttachmentReference color_ref{};
    color_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_ref.attachment = 0;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color_ref;

    VkRenderPassCreateInfo info{VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
    info.attachmentCount = 1;
    info.pAttachments = &color_attachment;
    info.subpassCount = 1;
    info.pSubpasses = &subpass;

    auto device = _scene->context()->device();
    if (device->Create(&info, &_render_pass) != VK_NULL_HANDLE) {
        panic("Failed to create render pass for view");
    }
}
} // namespace ars::render::vk