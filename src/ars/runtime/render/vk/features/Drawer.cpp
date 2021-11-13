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
                          uint32_t count,
                          const glm::mat4 *mvp_arr,
                          const uint32_t *ids,
                          const std::shared_ptr<Mesh> *meshes) const {
    assert(meshes != nullptr);
    assert(ids != nullptr);
    assert(mvp_arr != nullptr);

    _draw_id_pipeline->bind(cmd);

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

        cmd->PushConstants(_draw_id_pipeline->pipeline_layout(),
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
}

Drawer::Drawer(View *view) : _view(view) {
    init_render_pass();
    init_draw_id_pipeline();
    init_draw_id_billboard_alpha_clip();
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

void Drawer::init_draw_id_pipeline() {
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

    _draw_id_pipeline = std::make_unique<GraphicsPipeline>(ctx, info);
}

void Drawer::draw_ids_billboard_alpha_clip(
    CommandBuffer *cmd,
    uint32_t count,
    const glm::mat4 *mvp_arr,
    const uint32_t *ids,
    const Handle<Texture> *textures) const {
    assert(textures != nullptr);
    assert(ids != nullptr);
    assert(mvp_arr != nullptr);

    _draw_id_billboard_alpha_clip_pipeline->bind(cmd);

    for (int i = 0; i < count; i++) {
        auto texture = textures[i];
        if (texture == nullptr) {
            continue;
        }

        cmd->PushConstants(
            _draw_id_billboard_alpha_clip_pipeline->pipeline_layout(),
            VK_SHADER_STAGE_VERTEX_BIT,
            0,
            sizeof(glm::mat4),
            mvp_arr + i);

        DescriptorEncoder desc{};
        desc.set_texture(0, 0, texture.get());

        struct Param {
            uint32_t color_id;
            float alpha_clip;
        };
        Param param{};
        param.color_id = ids[i];
        param.alpha_clip = 0.1f;
        desc.set_buffer_data(0, 1, param);

        desc.commit(cmd, _draw_id_billboard_alpha_clip_pipeline.get());

        cmd->Draw(6, 1, 0, 0);
    }
}

RenderPass *Drawer::draw_id_render_pass() const {
    return _render_pass.get();
}

void Drawer::init_draw_id_billboard_alpha_clip() {
    auto ctx = _view->context();
    auto vert_shader = std::make_unique<Shader>(ctx, "Billboard.vert");
    auto frag_shader = std::make_unique<Shader>(ctx, "BillboardObjectId.frag");

    auto depth_stencil = enabled_depth_stencil_state();

    VkPushConstantRange push_constant_range = {
        VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4)};

    GraphicsPipelineInfo info{};
    info.shaders.push_back(vert_shader.get());
    info.shaders.push_back(frag_shader.get());
    info.render_pass = _render_pass.get();
    info.subpass = 0;
    info.depth_stencil = &depth_stencil;

    info.push_constant_range_count = 1;
    info.push_constant_ranges = &push_constant_range;

    _draw_id_billboard_alpha_clip_pipeline =
        std::make_unique<GraphicsPipeline>(ctx, info);
}
} // namespace ars::render::vk