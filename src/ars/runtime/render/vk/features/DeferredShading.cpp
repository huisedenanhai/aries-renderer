#include "DeferredShading.h"
#include "../Context.h"
#include "../Environment.h"
#include "../Lut.h"
#include "../Profiler.h"
#include "../Scene.h"

namespace ars::render::vk {
void DeferredShading::init_pipeline() {
    _pipeline = ComputePipeline::create(_view->context(), "ShadingPass.comp");
}

DeferredShading::DeferredShading(View *view) : _view(view) {
    init_pipeline();
}

void DeferredShading::execute(CommandBuffer *cmd, NamedRT final_color_rt) {
    ARS_PROFILER_SAMPLE_VK(cmd, "Deferred Shading", 0xFF771233);

    auto final_color = _view->render_target(final_color_rt);
    auto final_color_extent = final_color->info().extent;
    auto env = _view->environment_vk();

    _pipeline->bind(cmd);

    DescriptorEncoder desc{};

    desc.set_texture(0, 0, final_color.get());
    desc.set_texture(0, 1, _view->render_target(NamedRT_GBuffer0).get());
    desc.set_texture(0, 2, _view->render_target(NamedRT_GBuffer1).get());
    desc.set_texture(0, 3, _view->render_target(NamedRT_GBuffer2).get());
    desc.set_texture(0, 4, _view->render_target(NamedRT_GBuffer3).get());
    desc.set_texture(0, 5, _view->render_target(NamedRT_Depth).get());
    desc.set_texture(0, 6, _view->context()->lut()->brdf_lut().get());
    desc.set_texture(0, 7, env->irradiance_cube_map_vk().get());
    desc.set_texture(0, 8, env->hdr_texture_vk().get());

    struct ShadingParam {
        int32_t width;
        int32_t height;
        int32_t point_light_count;
        int32_t directional_light_count;
        glm::mat4 I_P;
        glm::mat4 I_V;
        glm::vec3 env_radiance_factor;
        int32_t cube_map_mip_count;
    };

    ShadingParam param{};
    param.width = static_cast<int32_t>(final_color_extent.width);
    param.height = static_cast<int32_t>(final_color_extent.height);
    param.point_light_count =
        static_cast<int32_t>(_view->vk_scene()->point_lights.size());
    param.directional_light_count =
        static_cast<int32_t>(_view->vk_scene()->directional_lights.size());
    param.env_radiance_factor = env->radiance();
    param.cube_map_mip_count =
        static_cast<int32_t>(env->irradiance_cube_map()->mip_levels());

    auto v_matrix = _view->view_matrix();
    auto p_matrix = _view->projection_matrix();

    param.I_P = glm::inverse(p_matrix);
    param.I_V = glm::inverse(v_matrix);
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

    desc.commit(cmd, _pipeline.get());

    _pipeline->local_size().dispatch(cmd, param.width, param.height, 1);
}

void DeferredShading::render(RenderGraph &rg, NamedRT final_color_rt) {
    rg.add_pass(
        [&](RenderGraphPassBuilder &builder) {
            builder.compute_shader_write(final_color_rt);

            NamedRT rts[5] = {
                NamedRT_GBuffer0,
                NamedRT_GBuffer1,
                NamedRT_GBuffer2,
                NamedRT_GBuffer3,
                NamedRT_Depth,
            };

            for (auto &rt : rts) {
                builder.compute_shader_read(rt);
            }
        },
        [this, final_color_rt](CommandBuffer *cmd) {
            execute(cmd, final_color_rt);
        });
}
} // namespace ars::render::vk