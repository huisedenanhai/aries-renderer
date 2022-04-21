#pragma once

#include "../Material.h"

namespace ars::render::vk {
std::shared_ptr<GraphicsPipeline>
create_draw_pipeline(Context *context,
                     MaterialPassInfo pass,
                     const std::vector<Shader *> &shaders,
                     VkPipelineRasterizationStateCreateInfo *raster = nullptr);

MaterialPassTemplate
create_material_pass_template(Context *context,
                              const MaterialInfo &mat_info,
                              const MaterialPassInfo &pass_info);

MaterialTemplate create_material_template(Context *context,
                                          const MaterialInfo &mat_info);
} // namespace ars::render::vk