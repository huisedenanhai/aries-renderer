#pragma once

#include "../IMaterial.h"
#include <ars/runtime/core/ResData.h>

namespace ars::render {
class IContext;

struct MaterialResMeta {
    MaterialType type = MaterialType::Error;
    DataSlice properties;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(MaterialResMeta, type, properties)
};

std::shared_ptr<IMaterial>
load_material(IContext *context, Resources *res, const ResData &data);

nlohmann::json serialize_material(IMaterial *material);
} // namespace ars::render