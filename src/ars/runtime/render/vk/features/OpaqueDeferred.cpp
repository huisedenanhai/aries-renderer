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
    geometry_pass(cmd);
    geometry_pass_barrier(cmd);
    shading_pass(cmd);
}

void OpaqueDeferred::geometry_pass(CommandBuffer *cmd) {
    auto ctx = _view->context();

    Framebuffer *fb = nullptr;
    {
        auto rts = geometry_pass_rt_names();
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

    _geometry_pass_pipeline->bind(cmd);

    fb->set_viewport_scissor(cmd);

    auto extent = fb->extent();
    auto &rd_objs = _view->vk_scene()->render_objects;
    auto w_div_h =
        static_cast<float>(extent.width) / static_cast<float>(extent.height);
    auto v_matrix = _view->view_matrix();
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

        Transform t{};
        t.MV = v_matrix * matrix;
        t.I_MV = glm::inverse(t.MV);
        t.P = p_matrix;

        struct MaterialParam {
            glm::vec4 base_color_factor;
            float metallic_factor;
            float roughness_factor;
            float normal_scale;
            float occlusion_strength;
            glm::vec3 emission_factor;
        };

        MaterialParam m{};
        m.base_color_factor = glm::vec4(1.0f);
        m.metallic_factor = 1.0f;
        m.roughness_factor = 1.0f;
        m.normal_scale = 1.0f;
        m.occlusion_strength = 1.0f;
        m.emission_factor = glm::vec3(1.0f);

        DescriptorEncoder desc{};
        desc.set_buffer_data(0, 0, t);
        desc.set_buffer_data(1, 0, m);

        Handle<Texture> images[5] = {
            material->base_color_tex.vk_texture(),
            material->metallic_roughness_tex.vk_texture(),
            material->normal_tex.vk_texture(),
            material->occlusion_tex.vk_texture(),
            material->emission_tex.vk_texture(),
        };
        for (int i = 0; i < std::size(images); i++) {
            desc.set_texture(1, i + 1, images[i].get());
        }

        desc.commit(cmd, _geometry_pass_pipeline.get());

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
}

void OpaqueDeferred::geometry_pass_barrier(const CommandBuffer *cmd) const {
    // Wait attachments
    auto rts = geometry_pass_rts();
    VkImageMemoryBarrier image_barriers[std::size(rts) + 1]{};

    // Color attachments
    for (int i = 0; i < std::size(rts); i++) {
        auto &rt = rts[i];
        auto &barrier = image_barriers[i];
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.subresourceRange = rt->subresource_range();
        barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        // The last attachment is depth stencil attachment
        if (i + 1 == std::size(rts)) {
            barrier.srcAccessMask =
                VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        }
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        barrier.oldLayout = rt->layout();
        barrier.newLayout = rt->layout();
        barrier.image = rt->image();
    }

    auto final_color = _view->render_target(NamedRT_FinalColor0);

    // Transfer final color attachment layout
    auto &final_color_barrier = image_barriers[std::size(rts)];
    final_color_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    final_color_barrier.subresourceRange = final_color->subresource_range();
    final_color_barrier.srcAccessMask = 0;
    final_color_barrier.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
    final_color_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    final_color_barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    final_color_barrier.image = final_color->image();

    cmd->PipelineBarrier(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                             VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
                             VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
                         VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                         0,
                         0,
                         nullptr,
                         0,
                         nullptr,
                         static_cast<uint32_t>(std::size(image_barriers)),
                         image_barriers);

    final_color->assure_layout(VK_IMAGE_LAYOUT_GENERAL);
}

void OpaqueDeferred::shading_pass(CommandBuffer *cmd) const {
    auto final_color = _view->render_target(NamedRT_FinalColor0);
    auto final_color_extent = final_color->info().extent;

    _shading_pass_pipeline->bind(cmd);

    int32_t size[2] = {static_cast<int>(final_color_extent.width),
                       static_cast<int>(final_color_extent.height)};
    cmd->PushConstants(_shading_pass_pipeline->pipeline_layout(),
                       VK_SHADER_STAGE_COMPUTE_BIT,
                       0,
                       2 * sizeof(int32_t),
                       size);

    DescriptorEncoder desc{};
    auto rts = geometry_pass_rts();
    for (int i = 0; i < std::size(rts); i++) {
        desc.set_texture(0, i, rts[i].get());
    }
    desc.set_texture(0, std::size(rts), final_color.get());

    struct ShadingParam {
        int32_t width;
        int32_t height;
        int32_t point_light_count;
        int32_t directional_light_count;
        glm::mat4 I_P;
    };

    ShadingParam param{};
    param.width = static_cast<int32_t>(final_color_extent.width);
    param.height = static_cast<int32_t>(final_color_extent.height);
    param.point_light_count =
        static_cast<int32_t>(_view->vk_scene()->point_lights.size());
    param.directional_light_count =
        static_cast<int32_t>(_view->vk_scene()->directional_lights.size());

    auto v_matrix = _view->view_matrix();
    auto w_div_h = static_cast<float>(final_color_extent.width) /
                   static_cast<float>(final_color_extent.height);
    auto p_matrix = _view->camera().projection_matrix(w_div_h);

    param.I_P = glm::inverse(p_matrix);

    desc.set_buffer_data(1, 0, param);

    struct alignas(16) PointLightData {
        glm::vec3 position;
        ARS_PADDING_FIELD(float);
        glm::vec3 color;
        float intensity;
    };

    std::vector<PointLightData> point_light_data{};
    {
        auto &point_light_soa = _view->vk_scene()->point_lights;
        point_light_data.resize(
            std::max(point_light_soa.size(), static_cast<size_t>(1)));

        auto xform_arr = point_light_soa.get_array<math::XformTRS<float>>();
        auto light_arr = point_light_soa.get_array<Light>();
        for (int i = 0; i < point_light_soa.size(); i++) {
            auto &data = point_light_data[i];
            data.position =
                math::transform_position(v_matrix, xform_arr[i].translation());
            data.color = light_arr[i].color;
            data.intensity = light_arr[i].intensity;
        }
    }
    desc.set_buffer_data(1, 1, point_light_data);

    struct alignas(16) DirectionalLightData {
        glm::vec3 direction;
        ARS_PADDING_FIELD(float);
        glm::vec3 color;
        float intensity;
    };

    std::vector<DirectionalLightData> dir_light_data{};
    {
        auto &dir_light_soa = _view->vk_scene()->directional_lights;
        dir_light_data.resize(
            std::max(dir_light_soa.size(), static_cast<size_t>(1)));

        auto xform_arr = dir_light_soa.get_array<math::XformTRS<float>>();
        auto light_arr = dir_light_soa.get_array<Light>();
        for (int i = 0; i < dir_light_soa.size(); i++) {
            auto &data = dir_light_data[i];
            data.direction = glm::normalize(
                math::transform_direction(v_matrix, xform_arr[i].forward()));
            data.color = light_arr[i].color;
            data.intensity = light_arr[i].intensity;
        }
    }
    desc.set_buffer_data(1, 2, dir_light_data);

    desc.commit(cmd, _shading_pass_pipeline.get());

    _shading_pass_pipeline->local_size().dispatch(
        cmd, param.width, param.height, 1);
}

void OpaqueDeferred::init_render_pass() {
    auto rts = geometry_pass_rt_names();
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

std::array<NamedRT, 5> OpaqueDeferred::geometry_pass_rt_names() const {
    // The last one must be the depth texture
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

std::array<Handle<Texture>, 5> OpaqueDeferred::geometry_pass_rts() const {
    auto rt_names = geometry_pass_rt_names();
    std::array<Handle<Texture>, 5> rts{};
    for (int i = 0; i < std::size(rts); i++) {
        rts[i] = _view->render_target(rt_names[i]);
    }
    return rts;
}

std::vector<PassDependency> OpaqueDeferred::dst_dependencies() {
    std::vector<PassDependency> deps(1);
    auto &d = deps[0];
    d.texture = _view->render_target(NamedRT_FinalColor0);
    d.access_mask = VK_ACCESS_SHADER_WRITE_BIT;
    d.layout = VK_IMAGE_LAYOUT_GENERAL;
    d.stage_mask = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;

    return deps;
}

} // namespace ars::render::vk