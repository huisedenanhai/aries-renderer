#include "DeferredShading.h"
#include "../Context.h"
#include "../Effect.h"
#include "../Lut.h"
#include "../Profiler.h"
#include "../Scene.h"
#include "../Sky.h"
#include "Renderer.h"

namespace ars::render::vk {
void DeferredShading::init_pipeline() {
    _lit_point_light_pipeline = create_pipeline("LIT_POINT_LIGHT");
    _lit_directional_light_pipeline = create_pipeline("LIT_DIRECTIONAL_LIGHT");
    _lit_sun_pipeline = create_pipeline("LIT_SUN");
    _lit_reflection_pipeline = create_pipeline("LIT_REFLECTION_EMISSION");
    _unlit_pipeline = create_pipeline("UNLIT");
}

DeferredShading::DeferredShading(View *view) : _view(view) {
    init_pipeline();
}

void DeferredShading::execute(CommandBuffer *cmd) {
    ARS_PROFILER_SAMPLE_VK(cmd, "Deferred Shading", 0xFF771233);

    auto ctx = _view->context();

    auto rp = render_pass().render_pass;
    auto fb = ctx->create_tmp_framebuffer(
        rp, {_view->render_target(NamedRT_LinearColor)});

    // clear color to zero
    VkClearValue clear_values[1]{};
    auto rp_exec = rp->begin(cmd, fb, clear_values, VK_SUBPASS_CONTENTS_INLINE);

    shade_unlit(cmd);
    shade_reflection_emission(cmd);

    auto physical_sky = dynamic_cast<PhysicalSky *>(
        _view->effect_vk()->background_vk()->sky_vk());

    // Special shade for sun in physical sky
    shade_directional_lights(cmd, physical_sky != nullptr);
    if (physical_sky != nullptr) {
        shade_sun(cmd, physical_sky);
    }

    shade_point_lights(cmd);

    rp->end(rp_exec);
}

void DeferredShading::render(RenderGraph &rg) {
    rg.add_pass(
        [&](RenderGraphPassBuilder &builder) {
            builder.access(NamedRT_LinearColor,
                           VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                           VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);

            auto frag_read = [&](auto tex) {
                builder.access(tex,
                               VK_ACCESS_SHADER_READ_BIT,
                               VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
            };

            frag_read(NamedRT_GBuffer0);
            frag_read(NamedRT_GBuffer1);
            frag_read(NamedRT_GBuffer2);
            frag_read(NamedRT_GBuffer3);
            frag_read(NamedRT_Depth);
            if (_view->effect()->screen_space_reflection()->enabled()) {
                frag_read(NamedRT_Reflection);
            }

            auto sky = _view->effect_vk()->background_vk()->sky_vk();
            frag_read(sky->data()->irradiance_cube_map());

            auto physical_sky = dynamic_cast<PhysicalSky *>(sky);
            if (physical_sky != nullptr) {
                frag_read(physical_sky->transmittance_lut());
            }

            // Read all shadow maps
            auto &dir_lights = _view->scene_vk()->directional_lights;
            auto shadow_arr =
                dir_lights.get_array<std::unique_ptr<ShadowMap>>();
            for (int i = 0; i < dir_lights.size(); i++) {
                auto sm = shadow_arr[i].get();
                if (sm != nullptr) {
                    frag_read(sm->texture());
                }
            }
        },
        [this](CommandBuffer *cmd) { execute(cmd); });
}

std::unique_ptr<GraphicsPipeline>
DeferredShading::create_pipeline(const std::string &flag) {
    auto ctx = _view->context();
    auto vert_shader = Shader::find_precompiled(ctx, "Blit.vert");
    auto frag_shader = Shader::find_precompiled(
        ctx, "Shading/ShadeMaterial.frag", {flag.c_str()});

    auto blend_additive =
        create_attachment_blend_state(VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ONE);
    VkPipelineColorBlendStateCreateInfo blend{
        VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};
    blend.attachmentCount = 1;
    blend.pAttachments = &blend_additive;

    GraphicsPipelineInfo info{};
    info.shaders.push_back(vert_shader.get());
    info.shaders.push_back(frag_shader.get());
    info.subpass = render_pass();
    info.blend = &blend;

    return std::make_unique<GraphicsPipeline>(ctx, info);
}

void DeferredShading::shade_unlit(CommandBuffer *cmd) {
    ARS_PROFILER_SAMPLE_VK(cmd, "Shade Unlit", 0xFF824851);
    set_up_shade(_unlit_pipeline.get(), cmd);
    cmd->Draw(3, 1, 0, 0);
}

void DeferredShading::set_up_shade(GraphicsPipeline *pipeline,
                                   CommandBuffer *cmd) {
    auto background = _view->effect_vk()->background_vk();
    auto sky = background->sky_vk()->data();

    pipeline->bind(cmd);

    DescriptorEncoder desc{};

    desc.set_texture(0, 0, _view->render_target(NamedRT_GBuffer0).get());
    desc.set_texture(0, 1, _view->render_target(NamedRT_GBuffer1).get());
    desc.set_texture(0, 2, _view->render_target(NamedRT_GBuffer2).get());
    desc.set_texture(0, 3, _view->render_target(NamedRT_GBuffer3).get());
    desc.set_texture(0, 4, _view->render_target(NamedRT_Depth).get());
    desc.set_buffer(0, 5, _view->transform_buffer().get());

    desc.commit(cmd, pipeline);
}

void DeferredShading::shade_reflection_emission(CommandBuffer *cmd) {
    ARS_PROFILER_SAMPLE_VK(cmd, "Shade Reflection Emission", 0xFF029405);
    set_up_shade(_lit_reflection_pipeline.get(), cmd);

    DescriptorEncoder desc{};
    auto ctx = _view->context();
    auto sky = _view->effect_vk()->background_vk()->sky_vk()->data();
    desc.set_texture(1, 1, ctx->lut()->brdf_lut().get());
    desc.set_texture(1, 2, sky->irradiance_cube_map().get());

    if (_view->effect()->screen_space_reflection()->enabled()) {
        desc.set_texture(1, 3, _view->render_target(NamedRT_Reflection).get());
    } else {
        desc.set_texture(
            1, 3, ctx->default_texture_vk(DefaultTexture::Zero).get());
    }

    struct ShadingParam {
        glm::vec3 env_radiance_factor;
        int32_t cube_map_mip_count;
    };

    ShadingParam param{};
    param.env_radiance_factor = sky->radiance();
    param.cube_map_mip_count =
        static_cast<int32_t>(sky->irradiance_cube_map()->info().mip_levels);

    desc.set_buffer_data(1, 0, param);

    desc.commit(cmd, _lit_reflection_pipeline.get());

    cmd->Draw(3, 1, 0, 0);
}

namespace {
struct alignas(16) DirectionalLightData {
    glm::vec3 direction;
    ARS_PADDING_FIELD(float);
    glm::vec3 color;
    float intensity;
};

DirectionalLightData
get_directional_light_data(View *view, Scene::DirectionalLights::Id id) {
    auto &soa = view->scene_vk()->directional_lights;
    auto v_matrix = view->view_matrix();

    DirectionalLightData data{};
    data.direction = -glm::normalize(math::transform_direction(
        v_matrix, soa.get<math::XformTRS<float>>(id).forward()));
    auto &light = soa.get<Light>(id);
    data.color = light.color;
    data.intensity = light.intensity;

    return data;
}

struct alignas(16) PointLightData {
    glm::vec3 position;
    ARS_PADDING_FIELD(float);
    glm::vec3 color;
    float intensity;
};

PointLightData get_point_light_data(View *view, Scene::PointLights::Id id) {
    auto &soa = view->scene_vk()->point_lights;
    auto v_matrix = view->view_matrix();

    PointLightData data{};
    data.position = math::transform_position(
        v_matrix, soa.get<math::XformTRS<float>>(id).translation());
    auto &light = soa.get<Light>(id);
    data.color = light.color;
    data.intensity = light.intensity;

    return data;
}

void set_up_directional_shadow(DescriptorEncoder &desc,
                               View *view,
                               Scene::DirectionalLights::Id id) {
    auto sm = view->scene_vk()
                  ->directional_lights.get<std::unique_ptr<ShadowMap>>(id)
                  .get();
    if (sm == nullptr) {
        return;
    }
    desc.set_buffer_data(2, 0, sm->data(view));
    desc.set_texture(2, 1, sm->texture().get());
}

} // namespace

void DeferredShading::shade_directional_lights(CommandBuffer *cmd,
                                               bool ignore_sun) {
    ARS_PROFILER_SAMPLE_VK(cmd, "Shade Directional Lights", 0xFF461741);
    auto scene = _view->scene_vk();
    auto light_count = scene->directional_lights.size();
    if (light_count == 0 ||
        (ignore_sun && light_count == 1 && scene->sun_id.valid())) {
        return;
    }

    set_up_shade(_lit_directional_light_pipeline.get(), cmd);

    scene->directional_lights.for_each_id([&](auto id) {
        if (ignore_sun && id == scene->sun_id) {
            return;
        }

        auto data = get_directional_light_data(_view, id);
        DescriptorEncoder desc{};
        desc.set_buffer_data(1, 0, data);
        set_up_directional_shadow(desc, _view, id);
        desc.commit(cmd, _lit_directional_light_pipeline.get());

        cmd->Draw(3, 1, 0, 0);
    });
}

void DeferredShading::shade_point_lights(CommandBuffer *cmd) {
    ARS_PROFILER_SAMPLE_VK(cmd, "Shade Point Lights", 0xFF113422);
    auto scene = _view->scene_vk();
    auto light_count = scene->point_lights.size();
    if (light_count == 0) {
        return;
    }

    set_up_shade(_lit_point_light_pipeline.get(), cmd);
    scene->point_lights.for_each_id([&](auto id) {
        auto data = get_point_light_data(_view, id);

        DescriptorEncoder desc{};
        desc.set_buffer_data(1, 0, data);
        desc.commit(cmd, _lit_point_light_pipeline.get());

        cmd->Draw(3, 1, 0, 0);
    });
}

void DeferredShading::shade_sun(CommandBuffer *cmd, PhysicalSky *sky) {
    ARS_PROFILER_SAMPLE_VK(cmd, "Shade Sun", 0xFF537181);
    auto scene = _view->scene_vk();
    if (!scene->sun_id.valid()) {
        return;
    }

    set_up_shade(_lit_sun_pipeline.get(), cmd);

    DescriptorEncoder desc{};

    auto data = get_directional_light_data(_view, scene->sun_id);
    desc.set_buffer_data(1, 0, data);
    desc.set_buffer(1, 1, sky->atmosphere_settings_buffer().get());
    desc.set_texture(1, 2, sky->transmittance_lut().get());
    set_up_directional_shadow(desc, _view, scene->sun_id);
    desc.commit(cmd, _lit_sun_pipeline.get());

    cmd->Draw(3, 1, 0, 0);
}

SubpassInfo DeferredShading::render_pass() {
    return _view->context()->renderer_data()->subpass(RenderPassID_Shading);
}
} // namespace ars::render::vk