#include "OpaqueGeometry.h"
#include "../Context.h"
#include "../Lut.h"
#include "../Material.h"
#include "../Mesh.h"
#include "../Profiler.h"
#include "../Scene.h"
#include "Drawer.h"
#include "Renderer.h"

namespace ars::render::vk {
OpaqueGeometry::OpaqueGeometry(View *view) : _view(view) {
    init_pipeline();
}

void OpaqueGeometry::execute(CommandBuffer *cmd) {
    ARS_PROFILER_SAMPLE_VK(cmd, "Opaque Geometry", 0xFF12AA34);

    auto ctx = _view->context();

    Framebuffer *fb = nullptr;
    auto rp = render_pass().render_pass;
    {
        auto rts = geometry_pass_rt_names();
        std::vector<Handle<Texture>> render_targets{};
        render_targets.reserve(rts.size());
        for (auto rt : rts) {
            render_targets.push_back(_view->render_target(rt));
        }
        fb = ctx->create_tmp_framebuffer(rp, std::move(render_targets));
    }

    // clear all rts to zero
    VkClearValue clear_values[5]{};
    auto rp_exec = rp->begin(cmd, fb, clear_values, VK_SUBPASS_CONTENTS_INLINE);

    auto draw_requests = _view->scene_vk()->gather_draw_requests();
    _view->drawer()->draw(cmd,
                          RenderPassID::Geometry,
                          static_cast<uint32_t>(draw_requests.size()),
                          draw_requests.data());

    //    _pipeline->bind(cmd);
    //
    //    auto &rd_objs = _view->scene_vk()->render_objects;
    //    auto v_matrix = _view->view_matrix();
    //    auto p_matrix = _view->projection_matrix();
    //    rd_objs.for_each_id([&](Scene::RenderObjects::Id id) {
    //        auto &matrix = rd_objs.get<glm::mat4>(id);
    //        auto &mesh = rd_objs.get<std::shared_ptr<Mesh>>(id);
    //        if (mesh == nullptr) {
    //            return;
    //        }
    //        auto &mat_untyped = rd_objs.get<std::shared_ptr<IMaterial>>(id);
    //        if (mat_untyped == nullptr ||
    //            mat_untyped->type() != MaterialType::MetallicRoughnessPBR) {
    //            return;
    //        }
    //        auto material =
    //            std::dynamic_pointer_cast<MetallicRoughnessMaterial>(mat_untyped);
    //
    //        struct Transform {
    //            glm::mat4 MV;
    //            glm::mat4 I_MV;
    //            glm::mat4 P;
    //        };
    //
    //        Transform t{};
    //        t.MV = v_matrix * matrix;
    //        t.I_MV = glm::inverse(t.MV);
    //        t.P = p_matrix;
    //
    //        struct MaterialParam {
    //            glm::vec4 base_color_factor;
    //            float metallic_factor;
    //            float roughness_factor;
    //            float normal_scale;
    //            float occlusion_strength;
    //            glm::vec3 emission_factor;
    //        };
    //
    //        MaterialParam m{};
    //        m.base_color_factor = material->base_color_factor;
    //        m.metallic_factor = material->metallic_factor;
    //        m.roughness_factor = material->roughness_factor;
    //        m.normal_scale = material->normal_scale;
    //        m.occlusion_strength = material->occlusion_strength;
    //        m.emission_factor = material->emission_factor;
    //
    //        DescriptorEncoder desc{};
    //        desc.set_buffer_data(0, 0, t);
    //        desc.set_buffer_data(1, 0, m);
    //
    //        Handle<Texture> images[5] = {
    //            material->base_color_tex.vk_texture(),
    //            material->metallic_roughness_tex.vk_texture(),
    //            material->normal_tex.vk_texture(),
    //            material->occlusion_tex.vk_texture(),
    //            material->emission_tex.vk_texture(),
    //        };
    //        for (int i = 0; i < std::size(images); i++) {
    //            desc.set_texture(1, i + 1, images[i].get());
    //        }
    //
    //        desc.commit(cmd, _pipeline.get());
    //
    //        VkBuffer vertex_buffers[] = {
    //            mesh->position_buffer()->buffer(),
    //            mesh->normal_buffer()->buffer(),
    //            mesh->tangent_buffer()->buffer(),
    //            mesh->tex_coord_buffer()->buffer(),
    //        };
    //        VkDeviceSize vertex_offsets[std::size(vertex_buffers)] = {};
    //        cmd->BindVertexBuffers(0,
    //                               static_cast<uint32_t>(std::size(vertex_buffers)),
    //                               vertex_buffers,
    //                               vertex_offsets);
    //        cmd->BindIndexBuffer(
    //            mesh->index_buffer()->buffer(), 0, VK_INDEX_TYPE_UINT32);
    //
    //        cmd->DrawIndexed(mesh->triangle_count() * 3, 1, 0, 0, 0);
    //    });

    rp->end(rp_exec);
}

void OpaqueGeometry::init_pipeline() {
    auto ctx = _view->context();
    auto vert_shader = Shader::find_precompiled(ctx, "GeometryPass.vert");
    auto frag_shader = Shader::find_precompiled(ctx, "GeometryPass.frag");

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
    info.subpass = render_pass();

    info.vertex_input = &vertex_input;
    info.depth_stencil = &depth_stencil;

    _pipeline = std::make_unique<GraphicsPipeline>(ctx, info);
}

std::array<NamedRT, 5> OpaqueGeometry::geometry_pass_rt_names() {
    // The last one must be the depth texture
    return {NamedRT_GBuffer0,
            NamedRT_GBuffer1,
            NamedRT_GBuffer2,
            NamedRT_GBuffer3,
            NamedRT_Depth};
}

void OpaqueGeometry::render(RenderGraph &rg) {
    rg.add_pass(
        [&](RenderGraphPassBuilder &builder) {
            for (auto rt : geometry_pass_rt_names()) {
                if (rt != NamedRT_Depth) {
                    builder.access(
                        rt,
                        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
                } else {
                    builder.access(
                        rt,
                        VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
                        VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
                            VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT);
                }
            }
        },
        [=](CommandBuffer *cmd) { execute(cmd); });
}

SubpassInfo OpaqueGeometry::render_pass() {
    return _view->context()->renderer_data()->subpass(RenderPassID::Geometry);
}
} // namespace ars::render::vk