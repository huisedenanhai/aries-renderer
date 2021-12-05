#pragma once

#include "../IMesh.h"
#include <ars/runtime/core/ResData.h>
#include <ars/runtime/core/Serde.h>

namespace ars::render {
struct MeshResMeta {
    math::AABB<float> aabb;
    DataSlice position;
    DataSlice normal;
    DataSlice tangent;
    DataSlice tex_coord;
    DataSlice indices;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(
        MeshResMeta, aabb, position, normal, tangent, tex_coord, indices)
};

std::shared_ptr<IMesh> load_mesh(IContext *context, const ResData &data);

} // namespace ars::render
