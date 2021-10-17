#include "OpaqueDeferred.h"
#include "../Context.h"
#include "../Material.h"
#include "../Mesh.h"
#include "../Scene.h"
#include "../View.h"
#include <ars/runtime/core/Log.h>

namespace ars::render::vk {
OpaqueDeferred::OpaqueDeferred(View *view) : _view(view) {
    init_render_pass();
    init_pipeline();
}

OpaqueDeferred::~OpaqueDeferred() {
    auto device = _view->context()->device();
    if (_render_pass != VK_NULL_HANDLE) {
        device->Destroy(_render_pass);
    }
}

void OpaqueDeferred::render(CommandBuffer *cmd) {
    auto ctx = _view->context();
    auto color_rt = _view->render_target(NamedRT_FinalColor);
    auto depth_rt = _view->render_target(NamedRT_Depth);
    VkExtent2D extent{
        color_rt->info().extent.width,
        color_rt->info().extent.height,
    };

    VkFramebufferCreateInfo fb_info{VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
    fb_info.width = extent.width;
    fb_info.height = extent.height;
    fb_info.renderPass = _render_pass;
    fb_info.layers = 1;

    VkImageView attachments[2] = {color_rt->image_view(),
                                  depth_rt->image_view()};
    fb_info.attachmentCount = static_cast<uint32_t>(std::size(attachments));
    fb_info.pAttachments = attachments;

    auto fb = ctx->create_tmp_framebuffer(&fb_info);
    VkClearValue clear_values[2]{};
    auto &clear_color = clear_values[0].color.float32;
    clear_color[0] = 1.0f;
    clear_color[1] = 0.0f;
    clear_color[2] = 0.0f;
    clear_color[3] = 1.0f;

    auto &clear_depth_stencil = clear_values[1].depthStencil;
    clear_depth_stencil.depth = 0.0f;

    VkRenderPassBeginInfo rp_begin{VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
    rp_begin.renderPass = _render_pass;
    rp_begin.framebuffer = fb;
    rp_begin.clearValueCount = static_cast<uint32_t>(std::size(clear_values));
    rp_begin.pClearValues = clear_values;
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

    auto &rd_objs = _view->vk_scene()->render_objects;
    auto w_div_h =
        static_cast<float>(extent.width) / static_cast<float>(extent.height);
    auto vp_matrix = _view->camera().projection_matrix(w_div_h) *
                     glm::inverse(_view->xform().matrix_no_scale());
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

        auto mvp = vp_matrix * matrix;
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
}

void OpaqueDeferred::init_render_pass() {
    VkAttachmentDescription attachments[2]{};

    auto &color_attachment = attachments[0];
    color_attachment.format = _view->rt_info(NamedRT_FinalColor).texture.format;
    color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    color_attachment.finalLayout = VK_IMAGE_LAYOUT_GENERAL;

    auto &depth_attachment = attachments[1];
    depth_attachment.format = _view->rt_info(NamedRT_Depth).texture.format;
    depth_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depth_attachment.finalLayout = VK_IMAGE_LAYOUT_GENERAL;

    VkAttachmentReference color_ref{};
    color_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_ref.attachment = 0;

    VkAttachmentReference depth_ref{};
    depth_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    depth_ref.attachment = 1;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color_ref;
    subpass.pDepthStencilAttachment = &depth_ref;

    VkRenderPassCreateInfo info{VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
    info.attachmentCount = static_cast<uint32_t>(std::size(attachments));
    info.pAttachments = attachments;
    info.subpassCount = 1;
    info.pSubpasses = &subpass;

    auto device = _view->context()->device();
    if (device->Create(&info, &_render_pass) != VK_NULL_HANDLE) {
        panic("Failed to create render pass for Opaque Deferred Pass");
    }
}

void OpaqueDeferred::init_pipeline() {
    auto ctx = _view->context();
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

    auto depth_stencil = enabled_depth_stencil_state();

    GraphicsPipelineInfo info{};
    info.shaders.push_back(vert_shader.get());
    info.shaders.push_back(frag_shader.get());
    info.render_pass = _render_pass;
    info.subpass = 0;

    info.push_constant_range_count = 1;
    info.push_constant_ranges = &push_constant_range;
    info.vertex_input = &vertex_input;
    info.depth_stencil = &depth_stencil;

    _base_color_pipeline = std::make_unique<GraphicsPipeline>(ctx, info);
}
} // namespace ars::render::vk