#pragma once

#include "../Material.h"

namespace ars::render::vk {
VkPipelineRasterizationStateCreateInfo
rasterization_state(const MaterialInfo &mat_info);

std::shared_ptr<GraphicsPipeline>
create_draw_pipeline(Context *context,
                     const MaterialPassInfo &pass,
                     const std::vector<Shader *> &shaders,
                     const VkPipelineRasterizationStateCreateInfo *raster);

std::shared_ptr<GraphicsPipeline>
create_draw_pipeline(Context *context,
                     const MaterialInfo &mat_info,
                     const MaterialPassInfo &pass_info,
                     const char *glsl_file,
                     VkShaderStageFlags stages,
                     const VkPipelineRasterizationStateCreateInfo *raster,
                     std::vector<const char *> common_flags);

MaterialPassTemplate
create_material_pass_template(Context *context,
                              const MaterialInfo &mat_info,
                              const MaterialPassInfo &pass_info);

MaterialTemplate create_material_template(Context *context,
                                          const MaterialInfo &mat_info);
} // namespace ars::render::vk