#include "View.h"
#include "Context.h"
#include "Material.h"
#include "Mesh.h"
#include "Pipeline.h"
#include "Scene.h"
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

    auto fb = ctx->create_tmp_framebuffer(&fb_info);
    ctx->queue()->submit_once([&](CommandBuffer *cmd) {
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

        cmd->BindPipeline(VK_PIPELINE_BIND_POINT_GRAPHICS,
                          _base_color_pipeline->pipeline());

        VkViewport viewport;
        viewport.x = 0;
        viewport.y = 0;
        viewport.width = (float)extent.width;
        viewport.height = (float)extent.height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        cmd->SetViewport(0, 1, &viewport);

        VkRect2D scissor{{0, 0}, extent};
        cmd->SetScissor(0, 1, &scissor);

        auto &rd_objs = _scene->render_objects;
        rd_objs.for_each_id([&](Scene::RenderObjects::Id id) {
            auto &matrix = rd_objs.get<glm::mat4>(id);
            auto &mesh = rd_objs.get<std::shared_ptr<Mesh>>(id);
            auto &material = rd_objs.get<std::shared_ptr<IMaterial>>(id);
            auto base_color = upcast(material->base_color_tex().get());

            auto desc_set = _base_color_pipeline->alloc_desc_set(0);
            VkWriteDescriptorSet write{};
            VkDescriptorImageInfo image_info{};
            fill_combined_image_sampler(
                &write, &image_info, desc_set, 0, base_color.get());

            ctx->device()->UpdateDescriptorSets(1, &write, 0, nullptr);

            cmd->BindDescriptorSets(VK_PIPELINE_BIND_POINT_GRAPHICS,
                                    _base_color_pipeline->pipeline_layout(),
                                    0,
                                    1,
                                    &desc_set,
                                    0,
                                    nullptr);

            auto mvp = matrix;
            cmd->PushConstants(_base_color_pipeline->pipeline_layout(),
                               VK_SHADER_STAGE_VERTEX_BIT,
                               0,
                               sizeof(glm::mat4),
                               &mvp);

            VkBuffer vertex_buffers[2] = {
                mesh->position_buffer()->buffer(),
                mesh->tex_coord_buffer()->buffer(),
            };
            VkDeviceSize vertex_offsets[2] = {0, 0};
            cmd->BindVertexBuffers(0, 2, vertex_buffers, vertex_offsets);
            cmd->BindIndexBuffer(
                mesh->index_buffer()->buffer(), 0, VK_INDEX_TYPE_UINT32);

            cmd->DrawIndexed(mesh->triangle_count() * 3, 1, 0, 0, 0);
        });

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
    init_pipeline();
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
    auto device = context()->device();
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

    auto device = context()->device();
    if (device->Create(&info, &_render_pass) != VK_NULL_HANDLE) {
        panic("Failed to create render pass for view");
    }
}

void View::init_pipeline() {
    auto ctx = context();
    auto vert_shader = std::make_unique<Shader>(ctx, "BaseColor.vert");
    auto frag_shader = std::make_unique<Shader>(ctx, "BaseColor.frag");

    VkPipelineVertexInputStateCreateInfo vertex_input{
        VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};

    VkVertexInputBindingDescription vert_bindings[2] = {
        {0,
         static_cast<uint32_t>(sizeof(glm::vec3)),
         VK_VERTEX_INPUT_RATE_VERTEX},
        {1,
         static_cast<uint32_t>(sizeof(glm::vec2)),
         VK_VERTEX_INPUT_RATE_VERTEX},
    };

    VkVertexInputAttributeDescription vert_attrs[2] = {
        {0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0},
        {1, 1, VK_FORMAT_R32G32_SFLOAT, 0},
    };

    vertex_input.vertexAttributeDescriptionCount =
        static_cast<uint32_t>(std::size(vert_attrs));
    vertex_input.pVertexAttributeDescriptions = vert_attrs;
    vertex_input.vertexBindingDescriptionCount =
        static_cast<uint32_t>(std::size(vert_bindings));
    vertex_input.pVertexBindingDescriptions = vert_bindings;

    VkPushConstantRange push_constant_range{
        VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4)};

    GraphicsPipelineInfo info{};
    info.shaders.push_back(vert_shader.get());
    info.shaders.push_back(frag_shader.get());
    info.render_pass = _render_pass;
    info.subpass = 0;

    info.push_constant_range_count = 1;
    info.push_constant_ranges = &push_constant_range;
    info.vertex_input = &vertex_input;

    _base_color_pipeline = std::make_unique<GraphicsPipeline>(ctx, info);
}

Context *View::context() const {
    return _scene->context();
}
} // namespace ars::render::vk