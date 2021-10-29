#include "OpaqueDeferred.h"
#include "../Context.h"
#include "../Material.h"
#include "../Mesh.h"
#include "../Scene.h"

namespace ars::render::vk {
OpaqueDeferred::OpaqueDeferred(View *view) : _view(view) {
    init_render_pass();
    init_geometry_pass_pipeline();
    init_shading_pass_pipeline();
}

void OpaqueDeferred::render(CommandBuffer *cmd) {
    auto ctx = _view->context();

    Framebuffer *fb = nullptr;
    {
        auto rts = geometry_pass_rts();
        std::vector<Handle<Texture>> render_targets{};
        render_targets.reserve(rts.size());
        for (auto rt : rts) {
            render_targets.push_back(_view->render_target(rt));
        }
        fb = ctx->create_tmp_framebuffer(_render_pass.get(),
                                         std::move(render_targets));
    }

    // clear all rts to zero
    VkClearValue clear_values[5]{};
    auto rp_exec =
        _render_pass->begin(cmd, fb, clear_values, VK_SUBPASS_CONTENTS_INLINE);

    cmd->BindPipeline(VK_PIPELINE_BIND_POINT_GRAPHICS,
                      _geometry_pass_pipeline->pipeline());

    fb->set_viewport_scissor(cmd);

    auto extent = fb->extent();
    auto &rd_objs = _view->vk_scene()->render_objects;
    auto w_div_h =
        static_cast<float>(extent.width) / static_cast<float>(extent.height);
    auto v_matrix = glm::inverse(_view->xform().matrix_no_scale());
    auto p_matrix = _view->camera().projection_matrix(w_div_h);
    rd_objs.for_each_id([&](Scene::RenderObjects::Id id) {
        auto &matrix = rd_objs.get<glm::mat4>(id);
        auto &mesh = rd_objs.get<std::shared_ptr<Mesh>>(id);
        auto &mat_untyped = rd_objs.get<std::shared_ptr<IMaterial>>(id);
        if (mat_untyped->type() != MaterialType::MetallicRoughnessPBR) {
            return;
        }
        auto material =
            std::dynamic_pointer_cast<MetallicRoughnessMaterial>(mat_untyped);

        struct Transform {
            glm::mat4 MV;
            glm::mat4 I_MV;
            glm::mat4 P;
        };

        auto trans_buf = ctx->create_buffer(sizeof(Transform),
                                            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                            VMA_MEMORY_USAGE_CPU_TO_GPU);

        trans_buf->map_once([&](void *ptr) {
            Transform t{};
            t.MV = v_matrix * matrix;
            t.I_MV = glm::inverse(t.MV);
            t.P = p_matrix;

            std::memcpy(ptr, &t, sizeof(Transform));
        });

        struct MaterialParam {
            glm::vec4 base_color_factor;
            float metallic_factor;
            float roughness_factor;
            float normal_scale;
            float occlusion_strength;
            glm::vec3 emission_factor;
        };

        auto mat_buf = ctx->create_buffer(sizeof(MaterialParam),
                                          VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                          VMA_MEMORY_USAGE_CPU_TO_GPU);

        mat_buf->map_once([](void *ptr) {
            MaterialParam m{};
            m.base_color_factor = glm::vec4(1.0f);
            m.metallic_factor = 1.0f;
            m.roughness_factor = 1.0f;
            m.normal_scale = 1.0f;
            m.occlusion_strength = 1.0f;
            m.emission_factor = glm::vec3(1.0f);

            std::memcpy(ptr, &m, sizeof(MaterialParam));
        });

        VkDescriptorSet desc_sets[2] = {
            _geometry_pass_pipeline->alloc_desc_set(0),
            _geometry_pass_pipeline->alloc_desc_set(1),
        };
        VkWriteDescriptorSet write[7]{};
        VkDescriptorImageInfo image_info[5]{};
        Handle<Texture> images[5] = {
            material->base_color_tex.vk_texture(),
            material->metallic_roughness_tex.vk_texture(),
            material->normal_tex.vk_texture(),
            material->occlusion_tex.vk_texture(),
            material->emission_tex.vk_texture(),
        };
        auto image_writes = write + 2;
        for (int i = 0; i < 5; i++) {
            fill_desc_combined_image_sampler(&image_writes[i],
                                             &image_info[i],
                                             desc_sets[1],
                                             i + 1,
                                             images[i].get());
        }

        VkDescriptorBufferInfo trans_buf_info;
        fill_desc_uniform_buffer(&write[0],
                                 &trans_buf_info,
                                 desc_sets[0],
                                 0,
                                 trans_buf->buffer(),
                                 0,
                                 trans_buf->size());

        VkDescriptorBufferInfo mat_buf_info;
        fill_desc_uniform_buffer(&write[1],
                                 &mat_buf_info,
                                 desc_sets[1],
                                 0,
                                 mat_buf->buffer(),
                                 0,
                                 mat_buf->size());

        ctx->device()->UpdateDescriptorSets(
            static_cast<uint32_t>(std::size(write)), write, 0, nullptr);

        cmd->BindDescriptorSets(VK_PIPELINE_BIND_POINT_GRAPHICS,
                                _geometry_pass_pipeline->pipeline_layout(),
                                0,
                                2,
                                desc_sets,
                                0,
                                nullptr);

        VkBuffer vertex_buffers[] = {
            mesh->position_buffer()->buffer(),
            mesh->normal_buffer()->buffer(),
            mesh->tangent_buffer()->buffer(),
            mesh->tex_coord_buffer()->buffer(),
        };
        VkDeviceSize vertex_offsets[std::size(vertex_buffers)] = {};
        cmd->BindVertexBuffers(0,
                               static_cast<uint32_t>(std::size(vertex_buffers)),
                               vertex_buffers,
                               vertex_offsets);
        cmd->BindIndexBuffer(
            mesh->index_buffer()->buffer(), 0, VK_INDEX_TYPE_UINT32);

        cmd->DrawIndexed(mesh->triangle_count() * 3, 1, 0, 0, 0);
    });

    _render_pass->end(rp_exec);

    auto final_color = _view->render_target(NamedRT_FinalColor);
    auto final_color_extent = final_color->info().extent;

    VkImageMemoryBarrier image_barrier{VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
    image_barrier.subresourceRange = final_color->subresource_range();
    image_barrier.srcAccessMask = 0;
    image_barrier.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
    image_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    image_barrier.image = final_color->image();

    cmd->PipelineBarrier(VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                         VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                         0,
                         0,
                         nullptr,
                         0,
                         nullptr,
                         1,
                         &image_barrier);

    final_color->assure_layout(VK_IMAGE_LAYOUT_GENERAL);

    cmd->BindPipeline(VK_PIPELINE_BIND_POINT_COMPUTE,
                      _shading_pass_pipeline->pipeline());
    int32_t size[2] = {static_cast<int>(final_color_extent.width),
                       static_cast<int>(final_color_extent.height)};
    cmd->PushConstants(_shading_pass_pipeline->pipeline_layout(),
                       VK_SHADER_STAGE_COMPUTE_BIT,
                       0,
                       2 * sizeof(int32_t),
                       size);

    auto desc_set = _shading_pass_pipeline->alloc_desc_set(0);
    VkWriteDescriptorSet write{};
    VkDescriptorImageInfo image_info{};
    fill_desc_storage_image(
        &write, &image_info, desc_set, 0, final_color.get());
    ctx->device()->UpdateDescriptorSets(1, &write, 0, nullptr);

    cmd->BindDescriptorSets(VK_PIPELINE_BIND_POINT_COMPUTE,
                            _shading_pass_pipeline->pipeline_layout(),
                            0,
                            1,
                            &desc_set,
                            0,
                            nullptr);

    auto local_size_x = 32u;
    auto local_size_y = 32u;
    auto group_x = (extent.width + local_size_x - 1) / local_size_x;
    auto group_y = (extent.height + local_size_y - 1) / local_size_y;
    cmd->Dispatch(group_x, group_y, 1);
}

void OpaqueDeferred::init_render_pass() {
    auto rts = geometry_pass_rts();
    _render_pass = _view->create_single_pass_render_pass(
        rts.data(), static_cast<uint32_t>(std::size(rts) - 1), rts.back());
}

void OpaqueDeferred::init_geometry_pass_pipeline() {
    auto ctx = _view->context();
    auto vert_shader = std::make_unique<Shader>(ctx, "GeometryPass.vert");
    auto frag_shader = std::make_unique<Shader>(ctx, "GeometryPass.frag");

    VkPipelineVertexInputStateCreateInfo vertex_input{
        VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};

    VkVertexInputBindingDescription vert_bindings[4] = {
        {0,
         static_cast<uint32_t>(sizeof(glm::vec3)),
         VK_VERTEX_INPUT_RATE_VERTEX},
        {1,
         static_cast<uint32_t>(sizeof(glm::vec3)),
         VK_VERTEX_INPUT_RATE_VERTEX},
        {2,
         static_cast<uint32_t>(sizeof(glm::vec4)),
         VK_VERTEX_INPUT_RATE_VERTEX},
        {3,
         static_cast<uint32_t>(sizeof(glm::vec2)),
         VK_VERTEX_INPUT_RATE_VERTEX},
    };

    VkVertexInputAttributeDescription vert_attrs[4] = {
        {0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0},
        {1, 1, VK_FORMAT_R32G32B32_SFLOAT, 0},
        {2, 2, VK_FORMAT_R32G32B32A32_SFLOAT, 0},
        {3, 3, VK_FORMAT_R32G32_SFLOAT, 0},
    };

    vertex_input.vertexAttributeDescriptionCount =
        static_cast<uint32_t>(std::size(vert_attrs));
    vertex_input.pVertexAttributeDescriptions = vert_attrs;
    vertex_input.vertexBindingDescriptionCount =
        static_cast<uint32_t>(std::size(vert_bindings));
    vertex_input.pVertexBindingDescriptions = vert_bindings;

    auto depth_stencil = enabled_depth_stencil_state();

    GraphicsPipelineInfo info{};
    info.shaders.push_back(vert_shader.get());
    info.shaders.push_back(frag_shader.get());
    info.render_pass = _render_pass.get();
    info.subpass = 0;

    info.vertex_input = &vertex_input;
    info.depth_stencil = &depth_stencil;

    _geometry_pass_pipeline = std::make_unique<GraphicsPipeline>(ctx, info);
}

std::array<NamedRT, 5> OpaqueDeferred::geometry_pass_rts() const {
    return {NamedRT_GBuffer0,
            NamedRT_GBuffer1,
            NamedRT_GBuffer2,
            NamedRT_GBuffer3,
            NamedRT_Depth};
}

void OpaqueDeferred::init_shading_pass_pipeline() {
    auto ctx = _view->context();
    auto shader = std::make_unique<Shader>(ctx, "ShadingPass.comp");
    ComputePipelineInfo info{};
    info.shader = shader.get();

    VkPushConstantRange range{};
    range.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    range.size = 2 * sizeof(int32_t);
    range.offset = 0;

    info.push_constant_range_count = 1;
    info.push_constant_ranges = &range;

    _shading_pass_pipeline = std::make_unique<ComputePipeline>(ctx, info);
}
} // namespace ars::render::vk