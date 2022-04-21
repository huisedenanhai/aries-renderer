#include "MaterialTemplate.h"
#include "../Context.h"
#include "../features/Drawer.h"
#include "MetallicRoughnessPBR.h"
#include "Unlit.h"

namespace ars::render::vk {
MaterialPassTemplate
create_material_pass_template(Context *context,
                              const MaterialInfo &mat_info,
                              const MaterialPassInfo &pass_info) {
    switch (mat_info.shading_model) {
    case MaterialShadingModel::Unlit:
        return create_unlit_material_pass_template(
            context, mat_info, pass_info);
    case MaterialShadingModel::MetallicRoughnessPBR:
        return create_metallic_roughness_pbr_material_pass_template(
            context, mat_info, pass_info);
    }
    return {};
}

MaterialTemplate create_material_template(Context *context,
                                          const MaterialInfo &mat_info) {
    MaterialTemplate t{};
    t.info = mat_info;
    for (int id = 0; id < RenderPassID_Count; id++) {
        MaterialPassInfo pass_info{};
        pass_info.pass_id = static_cast<RenderPassID>(id);

        pass_info.skinned = false;
        t.passes[pass_info.encode()] =
            create_material_pass_template(context, mat_info, pass_info);

        pass_info.skinned = true;
        t.passes[pass_info.encode()] =
            create_material_pass_template(context, mat_info, pass_info);
    }

    MaterialPropertyBlockInfo block{};
    block.name = "Material";

    std::set<std::string> existing_properties{};
    for (auto &p : t.passes) {
        if (p.property_layout == nullptr) {
            continue;
        }
        for (auto &prop : p.property_layout->info().properties) {
            if (existing_properties.count(prop.name) > 0) {
                continue;
            }
            existing_properties.insert(prop.name);
            block.properties.push_back(prop);
        }
    }

    t.property_layout =
        std::make_shared<MaterialPropertyBlockLayout>(context, block);
    return t;
}

std::shared_ptr<GraphicsPipeline>
create_draw_pipeline(Context *context,
                     const MaterialPassInfo &pass,
                     const std::vector<Shader *> &shaders,
                     VkPipelineRasterizationStateCreateInfo *raster) {
    VkPipelineVertexInputStateCreateInfo vertex_input{
        VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};

    std::vector<VkVertexInputBindingDescription> vert_bindings = {
        {0,
         static_cast<uint32_t>(sizeof(InstanceDrawParam)),
         VK_VERTEX_INPUT_RATE_INSTANCE},
        {1,
         static_cast<uint32_t>(sizeof(glm::vec3)),
         VK_VERTEX_INPUT_RATE_VERTEX},
        {2,
         static_cast<uint32_t>(sizeof(glm::vec3)),
         VK_VERTEX_INPUT_RATE_VERTEX},
        {3,
         static_cast<uint32_t>(sizeof(glm::vec4)),
         VK_VERTEX_INPUT_RATE_VERTEX},
        {4,
         static_cast<uint32_t>(sizeof(glm::vec2)),
         VK_VERTEX_INPUT_RATE_VERTEX},
    };

    if (pass.skinned) {
        vert_bindings.push_back({
            5,
            static_cast<uint32_t>(sizeof(glm::uvec4)),
            VK_VERTEX_INPUT_RATE_VERTEX,
        });
        vert_bindings.push_back({
            6,
            static_cast<uint32_t>(sizeof(glm::vec4)),
            VK_VERTEX_INPUT_RATE_VERTEX,
        });
    }

    std::vector<VkVertexInputAttributeDescription> vert_attrs = {
        {0, 0, VK_FORMAT_R32_UINT, offsetof(InstanceDrawParam, instance_id)},
        {1, 1, VK_FORMAT_R32G32B32_SFLOAT, 0},
        {2, 2, VK_FORMAT_R32G32B32_SFLOAT, 0},
        {3, 3, VK_FORMAT_R32G32B32A32_SFLOAT, 0},
        {4, 4, VK_FORMAT_R32G32_SFLOAT, 0},
    };

    if (pass.skinned) {
        vert_attrs.push_back({5, 5, VK_FORMAT_R32G32B32A32_UINT, 0});
        vert_attrs.push_back({6, 6, VK_FORMAT_R32G32B32A32_SFLOAT, 0});
    }

    vertex_input.vertexAttributeDescriptionCount =
        static_cast<uint32_t>(std::size(vert_attrs));
    vertex_input.pVertexAttributeDescriptions = std::data(vert_attrs);
    vertex_input.vertexBindingDescriptionCount =
        static_cast<uint32_t>(std::size(vert_bindings));
    vertex_input.pVertexBindingDescriptions = std::data(vert_bindings);

    auto depth_stencil = enabled_depth_stencil_state();

    GraphicsPipelineInfo info{};
    info.shaders = shaders;
    info.subpass = context->renderer_data()->subpass(pass.pass_id);

    info.vertex_input = &vertex_input;
    info.depth_stencil = &depth_stencil;
    info.raster = raster;

    return std::make_shared<GraphicsPipeline>(context, info);
}

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
                     const MaterialInfo &mat_info,
                     const MaterialPassInfo &pass_info,
                     const char *glsl_file,
                     VkShaderStageFlags stages,
                     VkPipelineRasterizationStateCreateInfo *raster,
                     std::vector<const char *> common_flags) {
    std::vector<std::unique_ptr<Shader>> unique_shaders{};

    if (pass_info.skinned) {
        common_flags.push_back("ARS_SKINNED");
    }
    if (mat_info.features & MaterialFeature_AlphaClipBit) {
        common_flags.push_back("ARS_MATERIAL_ALPHA_CLIP");
    }
    if (mat_info.features & MaterialFeature_DoubleSidedBit) {
        common_flags.push_back("ARS_MATERIAL_DOUBLE_SIDED");
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
} // namespace ars::render::vk