#include "MetallicRoughnessPBR.h"
#include "../Context.h"
#include "MaterialTemplate.h"

namespace ars::render::vk {
std::shared_ptr<MaterialPropertyBlockLayout>
create_metallic_roughness_pbr_material_property_layout(
    Context *context, const MaterialInfo &mat_info) {
    auto white_tex = context->default_texture(DefaultTexture::White);
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

    return std::make_shared<MaterialPropertyBlockLayout>(context, pbr);
}

namespace {
std::shared_ptr<GraphicsPipeline>
create_pbr_pass_pipeline(Context *context,
                         const MaterialInfo &mat_info,
                         const MaterialPassInfo &pass_info,
                         VkBool32 depth_bias_enable,
                         const std::vector<const char *> &common_flags) {

    auto raster = rasterization_state(mat_info);
    raster.depthBiasEnable = depth_bias_enable;

    return create_draw_pipeline(context,
                                mat_info,
                                pass_info,
                                "Draw/Uber.glsl",
                                VK_SHADER_STAGE_VERTEX_BIT |
                                    VK_SHADER_STAGE_FRAGMENT_BIT,
                                &raster,
                                common_flags);
}
} // namespace

std::shared_ptr<GraphicsPipeline>
create_metallic_roughness_pbr_material_pipeline(
    Context *context,
    const MaterialInfo &mat_info,
    const MaterialPassInfo &pass_info) {
    if (pass_info.pass_id == RenderPassID_Geometry) {
        return create_pbr_pass_pipeline(
            context, mat_info, pass_info, VK_FALSE, {"ARS_GEOMETRY_PASS"});
    }

    if (pass_info.pass_id == RenderPassID_Shadow) {
        return create_pbr_pass_pipeline(
            context, mat_info, pass_info, VK_TRUE, {"ARS_DEPTH_ONLY_PASS"});
    }

    if (pass_info.pass_id == RenderPassID_ObjectID) {
        return create_pbr_pass_pipeline(
            context, mat_info, pass_info, VK_FALSE, {"ARS_OBJECT_ID_PASS"});
    }

    return {};
}
} // namespace ars::render::vk