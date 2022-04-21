#include "MetallicRoughnessPBR.h"
#include "../Context.h"
#include "MaterialTemplate.h"

namespace ars::render::vk {
MaterialPassTemplate create_metallic_roughness_pbr_material_pass_template(
    Context *context,
    const MaterialInfo &mat_info,
    const MaterialPassInfo &pass_info) {
    auto white_tex = context->default_texture(DefaultTexture::White);

    if (pass_info.pass_id == RenderPassID_Geometry) {
        MaterialPassTemplate t{};
        MaterialPropertyBlockInfo pbr{};
        pbr.name = "MetallicRoughnessLit";
        pbr.add_property("base_color_factor", glm::vec4(1.0f));
        pbr.add_property("metallic_factor", 1.0f);
        pbr.add_property("roughness_factor", 1.0f);
        pbr.add_property("normal_scale", 1.0f);
        pbr.add_property("occlusion_strength", 1.0f);
        pbr.add_property("emission_factor", glm::vec3(0.0f));
        pbr.add_property("base_color_tex", white_tex);
        pbr.add_property("metallic_roughness_tex", white_tex);
        pbr.add_property("normal_tex",
                         context->default_texture(DefaultTexture::Normal));
        pbr.add_property("occlusion_tex", white_tex);
        pbr.add_property("emission_tex", white_tex);

        auto vert_shader =
            Shader::find_precompiled(context,
                                     "Draw/MetallicRoughnessPBR.glsl",
                                     {"FRILL_SHADER_STAGE_VERT"});
        auto frag_shader =
            Shader::find_precompiled(context,
                                     "Draw/MetallicRoughnessPBR.glsl",
                                     {"FRILL_SHADER_STAGE_FRAG"});

        t.property_layout =
            std::make_shared<MaterialPropertyBlockLayout>(context, pbr);
        t.pipeline = create_draw_pipeline(
            context, pass_info, {vert_shader.get(), frag_shader.get()});
        return t;
    }

    // Shadow pass
    if (pass_info.pass_id == RenderPassID_Shadow) {
        MaterialPassTemplate t{};
        auto vert_shader = Shader::find_precompiled(context, "Draw/Depth.vert");

        VkPipelineRasterizationStateCreateInfo raster{
            VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};
        raster.cullMode = VK_CULL_MODE_NONE;
        raster.lineWidth = 1.0f;
        raster.polygonMode = VK_POLYGON_MODE_FILL;
        raster.depthBiasEnable = VK_TRUE;

        t.property_layout = nullptr;
        t.pipeline = create_draw_pipeline(
            context, pass_info, {vert_shader.get()}, &raster);

        return t;
    }
    return {};
}
} // namespace ars::render::vk