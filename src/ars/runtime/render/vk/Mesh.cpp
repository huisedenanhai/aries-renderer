#include "Mesh.h"

namespace ars::render::vk {
void Mesh::set_position(glm::vec3 *positions,
                        size_t start_index,
                        size_t count) {}

void Mesh::set_normal(glm::vec3 *normals, size_t start_index, size_t count) {}

void Mesh::set_tangent(glm::vec4 *tangents, size_t start_index, size_t count) {}

void Mesh::set_tex_coord(glm::vec2 *tex_coord,
                         size_t start_index,
                         size_t count) {}

size_t Mesh::index_count() const {
    return 0;
}

void Mesh::set_index_count(size_t count) {}

Mesh::Mesh(const MeshInfo &info) : IMesh(info) {}
} // namespace ars::render::vk
