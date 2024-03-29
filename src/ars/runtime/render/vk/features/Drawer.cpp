#include "Drawer.h"
#include "../Context.h"
#include "../Profiler.h"

namespace ars::render::vk {
Drawer::Drawer(View *view) : _view(view) {
    init_draw_id_billboard_alpha_clip();
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
    return _view->context()
        ->renderer_data()
        ->subpass(RenderPassID_ObjectID)
        .render_pass;
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
    info.subpass.render_pass = draw_id_render_pass();
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
    std::vector<Handle<Texture>> ref_textures{};
    if (prop_block != nullptr) {
        auto prop_buf =
            ctx->create_buffer(prop_block->layout()->data_block_size(),
                               VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                               VMA_MEMORY_USAGE_CPU_TO_GPU);
        prop_buf->map_once([&](void *ptr) { prop_block->fill_data(ptr); });
        desc.set_buffer(0, 2, prop_buf.get());
        ref_textures = prop_block->referenced_textures();
    }

    constexpr uint32_t SAMPLER_2D_COUNT = 8;
    while (ref_textures.size() < SAMPLER_2D_COUNT) {
        ref_textures.push_back(ctx->default_texture_vk(DefaultTexture::White));
    }
    desc.set_textures(0, 3, ref_textures);

    if (req.skeleton != nullptr) {
        desc.set_buffer(1, 0, req.skeleton->joint_buffer().get());
    }

    desc.commit(cmd, pipeline);

    if (req.mesh != bound_req.mesh || req.skeleton != bound_req.skeleton) {
        std::vector<VkBuffer> vertex_buffers = {
            inst_buffer->buffer(),
        };
        std::vector<VkDeviceSize> vertex_offsets = {0};
        auto add_vert_buffer = [&](const HeapRange &buf) {
            vertex_buffers.push_back(buf.buffer());
            vertex_offsets.push_back(buf.offset);
        };
        add_vert_buffer(req.mesh->position_buffer());
        add_vert_buffer(req.mesh->normal_buffer());
        add_vert_buffer(req.mesh->tangent_buffer());
        add_vert_buffer(req.mesh->tex_coord_buffer());

        if (req.skeleton != nullptr) {
            add_vert_buffer(req.mesh->joint_buffer());
            add_vert_buffer(req.mesh->weight_buffer());
        }
        cmd->BindVertexBuffers(0,
                               static_cast<uint32_t>(std::size(vertex_buffers)),
                               vertex_buffers.data(),
                               vertex_offsets.data());
        auto index_buffer = req.mesh->index_buffer();
        cmd->BindIndexBuffer(
            index_buffer.buffer(), index_buffer.offset, VK_INDEX_TYPE_UINT32);
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

    {
        ARS_PROFILER_SAMPLE("Sort Request", 0xFF3813B5);
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
                          return lhs->material.pipeline <
                                 rhs->material.pipeline;
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
    }

    auto ctx = _view->context();

    Handle<Buffer> inst_buffer{};
    {
        ARS_PROFILER_SAMPLE("Alloc Instance Buffer", 0xFF384712);
        inst_buffer = ctx->create_buffer(sizeof(InstanceDrawParam) *
                                             sorted_requests.size(),
                                         VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
                                             VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                                         VMA_MEMORY_USAGE_CPU_TO_GPU);
    }

    {
        ARS_PROFILER_SAMPLE("Update Instance Buffer", 0xFF184FA1);
	inst_buffer->map_once([&](void* ptr) {
	    auto inst_data = reinterpret_cast<InstanceDrawParam *>(ptr);
            for (int i = 0; i < sorted_requests.size(); i++) {
                auto &inst = inst_data[i];
                auto req = sorted_requests[i];
		auto MV = V * req->M;
                inst.MV = MV;
                inst.I_MV = glm::inverse(MV);
                inst.material_id = 0;
                inst.instance_id = i;
                inst.custom_id = req->custom_id;
            }
	});
    }

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
