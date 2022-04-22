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
    if (count == 0) {
        return;
    }
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
    auto vert_shader = Shader::find_precompiled(ctx, "ObjectId.vert");
    auto frag_shader = Shader::find_precompiled(ctx, "ObjectId.frag");

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
    info.subpass.render_pass = _render_pass.get();
    info.subpass.index = 0;

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
    if (count == 0) {
        return;
    }
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
    auto vert_shader = Shader::find_precompiled(ctx, "Billboard.vert");
    auto frag_shader = Shader::find_precompiled(ctx, "BillboardObjectId.frag");

    auto depth_stencil = enabled_depth_stencil_state();

    VkPushConstantRange push_constant_range = {
        VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4)};

    GraphicsPipelineInfo info{};
    info.shaders.push_back(vert_shader.get());
    info.shaders.push_back(frag_shader.get());
    info.subpass.render_pass = _render_pass.get();
    info.subpass.index = 0;
    info.depth_stencil = &depth_stencil;

    info.push_constant_range_count = 1;
    info.push_constant_ranges = &push_constant_range;

    _draw_id_billboard_alpha_clip_pipeline =
        std::make_unique<GraphicsPipeline>(ctx, info);
}

void Drawer::draw(CommandBuffer *cmd,
                  const glm::mat4 &P,
                  const glm::mat4 &V,
                  ars::Span<const DrawRequest> requests,
                  const DrawCallbacks &callbacks) {
    if (requests.empty()) {
        return;
    }

    auto transform_buffer =
        cmd->context()->create_buffer(sizeof(ViewTransform),
                                      VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                      VMA_MEMORY_USAGE_CPU_TO_GPU);
    transform_buffer->set_data(ViewTransform::from_V_P(V, P));
    draw(cmd, P, V, transform_buffer, requests, callbacks);
}

void Drawer::draw(CommandBuffer *cmd,
                  View *view,
                  ars::Span<const DrawRequest> requests) {
    draw(cmd,
         view->projection_matrix(),
         view->view_matrix(),
         view->transform_buffer(),
         requests,
         {});
}

namespace {
bool can_be_batched(const DrawRequest &lhs, const DrawRequest &rhs) {
    return lhs.material.pipeline == rhs.material.pipeline &&
           lhs.material.property_block == rhs.material.property_block &&
           lhs.mesh == rhs.mesh && lhs.skeleton == rhs.skeleton;
}

void dispatch_batch(CommandBuffer *cmd,
                    const DrawRequest &req,
                    Buffer *view_buffer,
                    Buffer *inst_buffer,
                    uint32_t start_index,
                    uint32_t end_index,
                    const DrawRequest &bound_req,
                    const DrawCallbacks &callbacks) {
    if (end_index <= start_index) {
        return;
    }

    auto ctx = cmd->context();

    auto pipeline = req.material.pipeline;
    if (pipeline != bound_req.material.pipeline) {
        pipeline->bind(cmd);
        if (callbacks.on_pipeline_bound) {
            callbacks.on_pipeline_bound(cmd);
        }
    }

    DescriptorEncoder desc{};
    desc.set_buffer(0, 0, view_buffer);
    desc.set_buffer(0, 1, inst_buffer);
    // The property block can be null, if that happens the shader should not
    // declare material and texture bindings
    auto prop_block = req.material.property_block;
    if (prop_block != nullptr) {
        auto prop_buf =
            ctx->create_buffer(prop_block->layout()->data_block_size(),
                               VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                               VMA_MEMORY_USAGE_CPU_TO_GPU);
        prop_buf->map_once([&](void *ptr) { prop_block->fill_data(ptr); });
        desc.set_buffer(0, 2, prop_buf.get());
        auto ref_textures = prop_block->referenced_textures();
        // If the material does not reference any textures, it should not
        // bind to texture slot
        if (!ref_textures.empty()) {
            desc.set_textures(0, 3, prop_block->referenced_textures());
        }
    }

    desc.commit(cmd, pipeline);

    if (req.mesh != bound_req.mesh) {
        std::vector<VkBuffer> vertex_buffers = {
            inst_buffer->buffer(),
            req.mesh->position_buffer()->buffer(),
            req.mesh->normal_buffer()->buffer(),
            req.mesh->tangent_buffer()->buffer(),
            req.mesh->tex_coord_buffer()->buffer(),
        };
        if (req.skeleton != nullptr) {
            vertex_buffers.push_back(req.mesh->joint_buffer()->buffer());
            vertex_buffers.push_back(req.mesh->weight_buffer()->buffer());
        }
        std::vector<VkDeviceSize> vertex_offsets(std::size(vertex_buffers));
        cmd->BindVertexBuffers(0,
                               static_cast<uint32_t>(std::size(vertex_buffers)),
                               vertex_buffers.data(),
                               vertex_offsets.data());
        cmd->BindIndexBuffer(
            req.mesh->index_buffer()->buffer(), 0, VK_INDEX_TYPE_UINT32);
    }

    cmd->DrawIndexed(req.mesh->triangle_count() * 3,
                     end_index - start_index,
                     0,
                     0,
                     start_index);
}
} // namespace

void Drawer::draw(CommandBuffer *cmd,
                  const glm::mat4 &P,
                  const glm::mat4 &V,
                  const Handle<Buffer> &view_transform_buffer,
                  ars::Span<const DrawRequest> requests,
                  const DrawCallbacks &callbacks) {
    if (requests.empty()) {
        return;
    }

    auto count = requests.size();
    std::vector<const DrawRequest *> sorted_requests{};
    sorted_requests.reserve(count);
    for (int i = 0; i < count; i++) {
        if (requests[i].material.pipeline == nullptr ||
            requests[i].mesh == nullptr) {
            continue;
        }
        sorted_requests.push_back(&requests[i]);
    }

    std::sort(sorted_requests.begin(),
              sorted_requests.end(),
              [&](const DrawRequest *lhs, const DrawRequest *rhs) {
                  if (lhs->material.pipeline != rhs->material.pipeline) {
                      return lhs->material.pipeline < rhs->material.pipeline;
                  }
                  if (lhs->material.property_block !=
                      rhs->material.property_block) {
                      return lhs->material.property_block <
                             rhs->material.property_block;
                  }
                  if (lhs->mesh != rhs->mesh) {
                      return lhs->mesh < rhs->mesh;
                  }
                  if (lhs->skeleton != rhs->skeleton) {
                      return lhs->skeleton < rhs->skeleton;
                  }
                  return lhs < rhs;
              });

    auto ctx = _view->context();

    auto inst_buffer = ctx->create_buffer(
        sizeof(InstanceDrawParam) * sorted_requests.size(),
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
        VMA_MEMORY_USAGE_CPU_TO_GPU);

    inst_buffer->map_once([&](void *ptr) {
        auto inst_ptr = reinterpret_cast<InstanceDrawParam *>(ptr);
        for (int i = 0; i < sorted_requests.size(); i++) {
            auto &inst = inst_ptr[i];
            auto req = sorted_requests[i];
            inst.MV = V * req->M;
            inst.I_MV = glm::inverse(inst.MV);
            inst.material_id = 0;
            inst.instance_id = i;
        }
    });

    DrawRequest bound_req{};
    size_t last_flushed_index = 0;

    auto flush = [&](size_t end_index) {
        if (end_index <= last_flushed_index) {
            return;
        }
        auto req = sorted_requests[last_flushed_index];
        dispatch_batch(cmd,
                       *req,
                       view_transform_buffer.get(),
                       inst_buffer.get(),
                       last_flushed_index,
                       end_index,
                       bound_req,
                       callbacks);
        bound_req = *req;
        last_flushed_index = end_index;
    };

    for (int i = 0; i < sorted_requests.size(); i++) {
        auto req = sorted_requests[i];
        if (!can_be_batched(*req, *sorted_requests[last_flushed_index])) {
            flush(i);
        }
    }
    flush(sorted_requests.size());
}

void Drawer::draw(CommandBuffer *cmd, ars::Span<const DrawRequest> requests) {
    draw(cmd, _view, requests);
}
} // namespace ars::render::vk