#include "DeferredShading.h"
#include "../Context.h"
#include "../Lut.h"
#include "../Scene.h"

namespace ars::render::vk {
void DeferredShading::init_pipeline() {
    _pipeline = ComputePipeline::create(_view->context(), "ShadingPass.comp");
}

DeferredShading::DeferredShading(View *view, NamedRT final_color_rt)
    : _view(view), _final_color_rt(final_color_rt) {
    init_pipeline();
}

void DeferredShading::render(CommandBuffer *cmd) {
    auto final_color = _view->render_target(_final_color_rt);
    auto final_color_extent = final_color->info().extent;

    _pipeline->bind(cmd);

    DescriptorEncoder desc{};
    desc.set_texture(0, 0, _view->render_target(NamedRT_GBuffer0).get());
    desc.set_texture(0, 1, _view->render_target(NamedRT_GBuffer1).get());
    desc.set_texture(0, 2, _view->render_target(NamedRT_GBuffer2).get());
    desc.set_texture(0, 3, _view->render_target(NamedRT_GBuffer3).get());
    desc.set_texture(0, 4, _view->render_target(NamedRT_Depth).get());
    desc.set_texture(0, 5, _view->context()->lut()->brdf_lut().get());
    desc.set_texture(0, 6, final_color.get());

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

    desc.commit(cmd, _pipeline.get());

    _pipeline->local_size().dispatch(cmd, param.width, param.height, 1);
}

std::vector<PassDependency> DeferredShading::dst_dependencies() {
    std::vector<PassDependency> deps(1);
    auto &d = deps[0];
    d.texture = _view->render_target(_final_color_rt);
    d.access_mask = VK_ACCESS_SHADER_WRITE_BIT;
    d.layout = VK_IMAGE_LAYOUT_GENERAL;
    d.stage_mask = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;

    return deps;
}

std::vector<PassDependency> DeferredShading::src_dependencies() {
    std::vector<PassDependency> deps;
    deps.reserve(6);

    NamedRT rts[5] = {
        NamedRT_GBuffer0,
        NamedRT_GBuffer1,
        NamedRT_GBuffer2,
        NamedRT_GBuffer3,
        NamedRT_Depth,
    };

    for (auto &rt : rts) {
        PassDependency d{};
        d.texture = _view->render_target(rt);
        d.access_mask = VK_ACCESS_SHADER_READ_BIT;
        d.layout = VK_IMAGE_LAYOUT_GENERAL;
        d.stage_mask = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
        deps.push_back(d);
    }

    PassDependency d{};
    d.texture = _view->render_target(_final_color_rt);
    d.access_mask = VK_ACCESS_SHADER_WRITE_BIT;
    d.layout = VK_IMAGE_LAYOUT_GENERAL;
    d.stage_mask = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
    deps.push_back(d);

    return deps;
}
} // namespace ars::render::vk