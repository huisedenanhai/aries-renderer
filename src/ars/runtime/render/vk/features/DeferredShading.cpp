#include "DeferredShading.h"
#include "../Context.h"
#include "../Effect.h"
#include "../Lut.h"
#include "../Profiler.h"
#include "../Scene.h"
#include "../Sky.h"

namespace ars::render::vk {
void DeferredShading::init_pipeline() {
    _pipeline =
        ComputePipeline::create(_view->context(), "Shading/ShadeMaterial.comp");
}

DeferredShading::DeferredShading(View *view) : _view(view) {
    init_pipeline();
}

void DeferredShading::execute(CommandBuffer *cmd) {
    ARS_PROFILER_SAMPLE_VK(cmd, "Deferred Shading", 0xFF771233);

    auto final_color = _view->render_target(NamedRT_LinearColor);
    auto final_color_extent = final_color->info().extent;
    auto background = _view->effect_vk()->background_vk();
    auto sky = background->sky_vk()->data();
    auto ctx = _view->context();

    _pipeline->bind(cmd);

    DescriptorEncoder desc{};

    desc.set_texture(0, 0, final_color.get());
    desc.set_texture(0, 1, _view->render_target(NamedRT_GBuffer0).get());
    desc.set_texture(0, 2, _view->render_target(NamedRT_GBuffer1).get());
    desc.set_texture(0, 3, _view->render_target(NamedRT_GBuffer2).get());
    desc.set_texture(0, 4, _view->render_target(NamedRT_GBuffer3).get());
    desc.set_texture(0, 5, _view->render_target(NamedRT_Depth).get());
    desc.set_texture(0, 6, ctx->lut()->brdf_lut().get());
    desc.set_texture(0, 7, sky->irradiance_cube_map().get());

    if (_view->effect()->screen_space_reflection()->enabled()) {
        desc.set_texture(0, 8, _view->render_target(NamedRT_Reflection).get());
    } else {
        desc.set_texture(
            0, 8, ctx->default_texture_vk(DefaultTexture::Zero).get());
    }

    struct ShadingParam {
        glm::mat4 I_P;
        glm::mat4 I_V;
        glm::vec3 env_radiance_factor;
        int32_t cube_map_mip_count;

        int32_t point_light_count;
        int32_t directional_light_count;
        //        glm::vec3 background_factor;
    };

    ShadingParam param{};
    param.point_light_count =
        static_cast<int32_t>(_view->scene_vk()->point_lights.size());
    param.directional_light_count =
        static_cast<int32_t>(_view->scene_vk()->directional_lights.size());
    param.env_radiance_factor = sky->radiance();
    param.cube_map_mip_count =
        static_cast<int32_t>(sky->irradiance_cube_map()->info().mip_levels);

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
        auto &point_light_soa = _view->scene_vk()->point_lights;
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
        auto &dir_light_soa = _view->scene_vk()->directional_lights;
        dir_light_data.resize(
            std::max(dir_light_soa.size(), static_cast<size_t>(1)));

        auto xform_arr = dir_light_soa.get_array<math::XformTRS<float>>();
        auto light_arr = dir_light_soa.get_array<Light>();
        for (int i = 0; i < dir_light_soa.size(); i++) {
            auto &data = dir_light_data[i];
            data.direction = -glm::normalize(
                math::transform_direction(v_matrix, xform_arr[i].forward()));
            data.color = light_arr[i].color;
            data.intensity = light_arr[i].intensity;
        }
    }
    desc.set_buffer_data(1, 2, dir_light_data);

    desc.commit(cmd, _pipeline.get());

    _pipeline->local_size().dispatch(cmd, final_color->info().extent);
}

void DeferredShading::render(RenderGraph &rg) {
    rg.add_pass(
        [&](RenderGraphPassBuilder &builder) {
            builder.compute_shader_write(NamedRT_LinearColor);
            builder.compute_shader_read(NamedRT_GBuffer0);
            builder.compute_shader_read(NamedRT_GBuffer1);
            builder.compute_shader_read(NamedRT_GBuffer2);
            builder.compute_shader_read(NamedRT_GBuffer3);
            builder.compute_shader_read(NamedRT_Depth);
            if (_view->effect()->screen_space_reflection()->enabled()) {
                builder.compute_shader_read(NamedRT_Reflection);
            }
            builder.compute_shader_read(_view->effect_vk()
                                            ->background_vk()
                                            ->sky_vk()
                                            ->data()
                                            ->irradiance_cube_map());
        },
        [this](CommandBuffer *cmd) { execute(cmd); });
}
} // namespace ars::render::vk