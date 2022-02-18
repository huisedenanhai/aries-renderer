#include "Mesh.h"
#include <ars/runtime/core/Log.h>

namespace ars::render {
std::shared_ptr<IMesh> load_mesh(IContext *context, const ResData &data) {
    if (!data.is_type<IMesh>()) {
        ARS_LOG_ERROR("Failed to load mesh: invalid data type");
        return nullptr;
    }

    MeshResMeta meta{};
    data.meta.get_to(meta);
    MeshInfo info{};
    auto round_up = [&](uint64_t a, uint64_t b) { return (a + b - 1) / b; };
    info.vertex_capacity = round_up(meta.position.size, sizeof(glm::vec3));
    info.triangle_capacity = round_up(meta.indices.size, sizeof(glm::u32vec3));
    auto mesh = context->create_mesh(info);
    mesh->set_aabb(meta.aabb);

#define ARS_MESH_SET_ATTR(m, ty, attr)                                         \
    mesh->m(reinterpret_cast<const ty *>(&data.data[meta.attr.offset]),        \
            0,                                                                 \
            meta.attr.size / sizeof(ty))

    ARS_MESH_SET_ATTR(set_position, glm::vec3, position);
    ARS_MESH_SET_ATTR(set_normal, glm::vec3, normal);
    ARS_MESH_SET_ATTR(set_tangent, glm::vec4, tangent);
    ARS_MESH_SET_ATTR(set_tex_coord, glm::vec2, tex_coord);
    ARS_MESH_SET_ATTR(set_indices, glm::u32vec3, indices);

#undef ARS_MESH_SET_ATTR

    mesh->set_triangle_count(meta.indices.size / sizeof(glm::u32vec3));
    return mesh;
}
} // namespace ars::render