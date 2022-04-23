#include "MetallicRoughnessPBR.h"
#include "../Context.h"
#include "MaterialTemplate.h"

namespace ars::render::vk {
namespace {
MaterialPassTemplate
create_pbr_geometry_pass_template(Context *context,
                                  const MaterialInfo &mat_info,
                                  const MaterialPassInfo &pass_info) {
    auto white_tex = context->default_texture(DefaultTexture::White);
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

    if (mat_info.features & MaterialFeature_AlphaClipBit) {
        pbr.add_property("alpha_cutoff", 0.5f);
    }

    auto raster = rasterization_state(mat_info);

    t.property_layout =
        std::make_shared<MaterialPropertyBlockLayout>(context, pbr);
    t.pipeline = create_draw_pipeline(
        context,
        mat_info,
        pass_info,
        "Draw/Uber.glsl",
        VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
        &raster,
        {"ARS_METALLIC_ROUGHNESS_PBR_GEOMETRY_PASS"});
    return t;
}

MaterialPassTemplate
create_pbr_depth_pass_template(Context *context,
                               const MaterialInfo &mat_info,
                               const MaterialPassInfo &pass_info) {
    MaterialPassTemplate t{};

    auto raster = rasterization_state(mat_info);
    raster.depthBiasEnable = VK_TRUE;

    t.property_layout = nullptr;
    if (mat_info.features & MaterialFeature_AlphaClipBit) {
        MaterialPropertyBlockInfo block{};
        block.add_property("base_color_factor", glm::vec4(1.0f));
        block.add_property("base_color_tex",
                           context->default_texture(DefaultTexture::White));
        block.add_property("alpha_cutoff", 0.5f);
        t.property_layout =
            std::make_shared<MaterialPropertyBlockLayout>(context, block);
    }

    t.pipeline = create_draw_pipeline(context,
                                      mat_info,
                                      pass_info,
                                      "Draw/Uber.glsl",
                                      VK_SHADER_STAGE_VERTEX_BIT |
                                          VK_SHADER_STAGE_FRAGMENT_BIT,
                                      &raster,
                                      {"ARS_DEPTH_ONLY_PASS"});

    return t;
}
} // namespace

MaterialPassTemplate create_metallic_roughness_pbr_material_pass_template(
    Context *context,
    const MaterialInfo &mat_info,
    const MaterialPassInfo &pass_info) {

    if (pass_info.pass_id == RenderPassID_Geometry) {
        return create_pbr_geometry_pass_template(context, mat_info, pass_info);
    }

    if (pass_info.pass_id == RenderPassID_Shadow) {
        return create_pbr_depth_pass_template(context, mat_info, pass_info);
    }

    return {};
}
} // namespace ars::render::vk