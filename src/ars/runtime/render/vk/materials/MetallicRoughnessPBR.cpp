#include "MetallicRoughnessPBR.h"
#include "../Context.h"
#include "MaterialTemplate.h"

namespace ars::render::vk {
namespace {
VkPipelineRasterizationStateCreateInfo
rasterization_state(const MaterialInfo &mat_info) {
    VkPipelineRasterizationStateCreateInfo raster{
        VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};
    raster.lineWidth = 1.0f;
    raster.polygonMode = VK_POLYGON_MODE_FILL;

    raster.cullMode = VK_CULL_MODE_BACK_BIT;
    if (mat_info.features & MaterialFeature_DoubleSidedBit) {
        raster.cullMode = VK_CULL_MODE_NONE;
    }
    return raster;
}

std::shared_ptr<GraphicsPipeline>
create_draw_pipeline(Context *context,
                     const MaterialPassInfo &pass_info,
                     const char *glsl_file,
                     VkShaderStageFlags stages,
                     VkPipelineRasterizationStateCreateInfo *raster,
                     std::vector<const char *> common_flags = {}) {
    std::vector<std::unique_ptr<Shader>> unique_shaders{};

    if (pass_info.skinned) {
        common_flags.push_back("ARS_SKINNED");
    }

    if (stages & VK_SHADER_STAGE_VERTEX_BIT) {
        auto flags = common_flags;
        flags.push_back("FRILL_SHADER_STAGE_VERT");
        unique_shaders.emplace_back(
            Shader::find_precompiled(context, glsl_file, flags));
    }

    if (stages & VK_SHADER_STAGE_FRAGMENT_BIT) {
        auto flags = common_flags;
        flags.push_back("FRILL_SHADER_STAGE_FRAG");
        unique_shaders.emplace_back(
            Shader::find_precompiled(context, glsl_file, flags));
    }

    std::vector<Shader *> shaders{};
    for (auto &s : unique_shaders) {
        shaders.push_back(s.get());
    }

    return create_draw_pipeline(context, pass_info, shaders, raster);
}

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

    auto raster = rasterization_state(mat_info);

    t.property_layout =
        std::make_shared<MaterialPropertyBlockLayout>(context, pbr);
    t.pipeline = create_draw_pipeline(
        context,
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
    t.pipeline = create_draw_pipeline(context,
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