#pragma once

#include "../Material.h"

namespace ars::render::vk {
MaterialPassTemplate
create_unlit_material_pass_template(Context *context,
                                    const MaterialInfo &mat_info,
                                    const MaterialPassInfo &pass_info);
}