#pragma once

#include "../Material.h"

namespace ars::render::vk {
std::shared_ptr<GraphicsPipeline>
create_unlit_material_pipeline(Context *context,
                               const MaterialInfo &mat_info,
                               const MaterialPassInfo &pass_info);

std::shared_ptr<MaterialPropertyBlockLayout>
create_unlit_material_property_layout(Context *context,
                                      const MaterialInfo &mat_info);
} // namespace ars::render::vk