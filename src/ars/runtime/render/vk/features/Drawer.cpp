#include "Drawer.h"
#include "../Context.h"
#include "../View.h"

namespace ars::render::vk {
namespace {
struct PushConstant {
    glm::mat4 MVP;
    uint32_t color_id;
};
} // namespace

void vk::Drawer::draw_ids(CommandBuffer *cmd,
                          const Handle<Texture> &id_color,
                          const Handle<Texture> &depth,
                          uint32_t count,
                          const glm::mat4 *mvp_arr,
                          const uint32_t *ids,
                          const std::shared_ptr<Mesh> *meshes) const {
    assert(meshes != nullptr);
    assert(ids != nullptr);
    assert(mvp_arr != nullptr);

    auto ctx = _view->context();
    auto fb =
        ctx->create_tmp_framebuffer(_render_pass.get(), {id_color, depth});

    // Clear all rts to zero
    VkClearValue clear_values[5]{};
    auto rp_exec =
        _render_pass->begin(cmd, fb, clear_values, VK_SUBPASS_CONTENTS_INLINE);

    _pipeline->bind(cmd);
    fb->set_viewport_scissor(cmd);

    for (int i = 0; i < count; i++) {
        auto &mesh = meshes[i];
        if (mesh == nullptr) {
            return;
        }

        VkBuffer vertex_buffers[] = {
            mesh->position_buffer()->buffer(),
        };
        VkDeviceSize vertex_offsets[std::size(vertex_buffers)] = {};

        PushConstant push_constant{};
        push_constant.MVP = mvp_arr[i];
        push_constant.color_id = ids[i];

        cmd->PushConstants(_pipeline->pipeline_layout(),
                           VK_SHADER_STAGE_VERTEX_BIT |
                               VK_SHADER_STAGE_FRAGMENT_BIT,
                           0,
                           sizeof(PushConstant),
                           &push_constant);
        cmd->BindVertexBuffers(0,
                               static_cast<uint32_t>(std::size(vertex_buffers)),
                               vertex_buffers,
                               vertex_offsets);
        cmd->BindIndexBuffer(
            mesh->index_buffer()->buffer(), 0, VK_INDEX_TYPE_UINT32);
        cmd->DrawIndexed(mesh->triangle_count() * 3, 1, 0, 0, 0);
    }

    _render_pass->end(rp_exec);
}

Drawer::Drawer(View *view) : _view(view) {
    init_render_pass();
    init_pipeline();
}

void Drawer::init_render_pass() {
    RenderPassAttachmentInfo color_info{};
    color_info.format = ID_COLOR_ATTACHMENT_FORMAT;
    color_info.samples = VK_SAMPLE_COUNT_1_BIT;

    RenderPassAttachmentInfo depth_info{};
    depth_info.format = ID_DEPTH_STENCIL_ATTACHMENT_FORMAT;
    depth_info.samples = VK_SAMPLE_COUNT_1_BIT;

    _render_pass = RenderPass::create_with_single_pass(
        _view->context(), 1, &color_info, &depth_info);
}

void Drawer::init_pipeline() {
    auto ctx = _view->context();
    auto vert_shader = std::make_unique<Shader>(ctx, "ObjectId.vert");
    auto frag_shader = std::make_unique<Shader>(ctx, "ObjectId.frag");

    VkPipelineVertexInputStateCreateInfo vertex_input{
        VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};

    VkVertexInputBindingDescription vert_bindings[1] = {
        {0,
         static_cast<uint32_t>(sizeof(glm::vec3)),
         VK_VERTEX_INPUT_RATE_VERTEX},
    };

    VkVertexInputAttributeDescription vert_attrs[1] = {
        {0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0},
    };

    vertex_input.vertexAttributeDescriptionCount =
        static_cast<uint32_t>(std::size(vert_attrs));
    vertex_input.pVertexAttributeDescriptions = vert_attrs;
    vertex_input.vertexBindingDescriptionCount =
        static_cast<uint32_t>(std::size(vert_bindings));
    vertex_input.pVertexBindingDescriptions = vert_bindings;

    auto depth_stencil = enabled_depth_stencil_state();

    VkPushConstantRange push_constant_range = {VK_SHADER_STAGE_VERTEX_BIT |
                                                   VK_SHADER_STAGE_FRAGMENT_BIT,
                                               0,
                                               sizeof(PushConstant)};

    GraphicsPipelineInfo info{};
    info.shaders.push_back(vert_shader.get());
    info.shaders.push_back(frag_shader.get());
    info.render_pass = _render_pass.get();
    info.subpass = 0;

    info.vertex_input = &vertex_input;
    info.depth_stencil = &depth_stencil;

    info.push_constant_range_count = 1;
    info.push_constant_ranges = &push_constant_range;

    _pipeline = std::make_unique<GraphicsPipeline>(ctx, info);
}
} // namespace ars::render::vk