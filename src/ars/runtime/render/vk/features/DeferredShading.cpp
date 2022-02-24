#include "DeferredShading.h"
#include "../Context.h"
#include "../Effect.h"
#include "../Lut.h"
#include "../Profiler.h"
#include "../Scene.h"
#include "../Sky.h"

namespace ars::render::vk {
void DeferredShading::init_pipeline() {
    _lit_point_light_pipeline = create_pipeline("LIT_POINT_LIGHT");
    _lit_directional_light_pipeline = create_pipeline("LIT_DIRECTIONAL_LIGHT");
    _lit_sun_pipeline = create_pipeline("LIT_SUN");
    _lit_reflection_pipeline = create_pipeline("LIT_REFLECTION_EMISSION");
    _unlit_pipeline = create_pipeline("UNLIT");
}

DeferredShading::DeferredShading(View *view) : _view(view) {
    init_render_pass();
    init_pipeline();
}

void DeferredShading::execute(CommandBuffer *cmd) {
    ARS_PROFILER_SAMPLE_VK(cmd, "Deferred Shading", 0xFF771233);

    auto ctx = _view->context();

    auto fb = ctx->create_tmp_framebuffer(
        _render_pass.get(), {_view->render_target(NamedRT_LinearColor)});

    // clear color to zero
    VkClearValue clear_values[1]{};
    auto rp_exec =
        _render_pass->begin(cmd, fb, clear_values, VK_SUBPASS_CONTENTS_INLINE);

    shade_unlit(cmd);
    shade_reflection_emission(cmd);
    shade_directional_light(cmd);
    shade_point_light(cmd);
    shade_sun(cmd);

    _render_pass->end(rp_exec);
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
            frag_read(_view->effect_vk()
                          ->background_vk()
                          ->sky_vk()
                          ->data()
                          ->irradiance_cube_map());
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
    info.render_pass = _render_pass.get();
    info.subpass = 0;
    info.blend = &blend;

    return std::make_unique<GraphicsPipeline>(ctx, info);
}

void DeferredShading::init_render_pass() {
    RenderPassAttachmentInfo color{};

    color.format = VK_FORMAT_B10G11R11_UFLOAT_PACK32;
    color.samples = VK_SAMPLE_COUNT_1_BIT;
    color.load_op = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color.store_op = VK_ATTACHMENT_STORE_OP_STORE;
    color.initial_layout = VK_IMAGE_LAYOUT_UNDEFINED;
    color.final_layout = VK_IMAGE_LAYOUT_GENERAL;

    _render_pass = RenderPass::create_with_single_pass(
        _view->context(), 1, &color, nullptr);
}

void DeferredShading::shade_unlit(CommandBuffer *cmd) {
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

    //    auto v_matrix = _view->view_matrix();
    //
    //    struct alignas(16) PointLightData {
    //        glm::vec3 position;
    //        ARS_PADDING_FIELD(float);
    //        glm::vec3 color;
    //        float intensity;
    //    };
    //
    //    std::vector<PointLightData> point_light_data{};
    //    {
    //        auto &point_light_soa = _view->scene_vk()->point_lights;
    //        point_light_data.resize(
    //            std::max(point_light_soa.size(), static_cast<size_t>(1)));
    //
    //        auto xform_arr =
    //        point_light_soa.get_array<math::XformTRS<float>>(); auto light_arr
    //        = point_light_soa.get_array<Light>(); for (int i = 0; i <
    //        point_light_soa.size(); i++) {
    //            auto &data = point_light_data[i];
    //            data.position =
    //                math::transform_position(v_matrix,
    //                xform_arr[i].translation());
    //            data.color = light_arr[i].color;
    //            data.intensity = light_arr[i].intensity;
    //        }
    //    }
    //
    //    struct alignas(16) DirectionalLightData {
    //        glm::vec3 direction;
    //        ARS_PADDING_FIELD(float);
    //        glm::vec3 color;
    //        float intensity;
    //    };
    //
    //    std::vector<DirectionalLightData> dir_light_data{};
    //    {
    //        auto &dir_light_soa = _view->scene_vk()->directional_lights;
    //        dir_light_data.resize(
    //            std::max(dir_light_soa.size(), static_cast<size_t>(1)));
    //
    //        auto xform_arr = dir_light_soa.get_array<math::XformTRS<float>>();
    //        auto light_arr = dir_light_soa.get_array<Light>();
    //        for (int i = 0; i < dir_light_soa.size(); i++) {
    //            auto &data = dir_light_data[i];
    //            data.direction = -glm::normalize(
    //                math::transform_direction(v_matrix,
    //                xform_arr[i].forward()));
    //            data.color = light_arr[i].color;
    //            data.intensity = light_arr[i].intensity;
    //        }
    //    }
}

void DeferredShading::shade_reflection_emission(CommandBuffer *cmd) {
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

void DeferredShading::shade_directional_light(CommandBuffer *cmd) {
    //    set_up_shade(_lit_directional_light_pipeline.get(), cmd);
}

void DeferredShading::shade_point_light(CommandBuffer *cmd) {}

void DeferredShading::shade_sun(CommandBuffer *cmd) {}
} // namespace ars::render::vk