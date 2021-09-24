#include "Mesh.h"
#include "Buffer.h"
#include "Context.h"

namespace ars::render::vk {
Mesh::Mesh(Context *context, const MeshInfo &info)
    : IMesh(info), _context(context) {
    auto create_buffer = [&](VkDeviceSize size, VkBufferUsageFlags usage) {
        return _context->create_buffer(
            size, usage, VMA_MEMORY_USAGE_CPU_TO_GPU);
    };

    VkBufferUsageFlags vertex_buffer_usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    _position_buffer = create_buffer(info.vertex_capacity * sizeof(glm::vec3),
                                     vertex_buffer_usage);
    _normal_buffer = create_buffer(info.vertex_capacity * sizeof(glm::vec3),
                                   vertex_buffer_usage);
    _tangent_buffer = create_buffer(info.vertex_capacity * sizeof(glm::vec4),
                                    vertex_buffer_usage);
    _tex_coord_buffer = create_buffer(info.vertex_capacity * sizeof(glm::vec2),
                                      vertex_buffer_usage);

    VkBufferUsageFlags index_buffer_usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    _index_buffer = create_buffer(
        _info.triangle_capacity * sizeof(glm::u32vec3), index_buffer_usage);
}

void Mesh::set_position(glm::vec3 *positions,
                        size_t elem_offset,
                        size_t elem_count) {
    _position_buffer->set_data(positions, elem_offset, elem_count);
}

void Mesh::set_normal(glm::vec3 *normals,
                      size_t elem_offset,
                      size_t elem_count) {
    _normal_buffer->set_data(normals, elem_offset, elem_count);
}

void Mesh::set_tangent(glm::vec4 *tangents,
                       size_t elem_offset,
                       size_t elem_count) {
    _tangent_buffer->set_data(tangents, elem_offset, elem_count);
}

void Mesh::set_tex_coord(glm::vec2 *tex_coord,
                         size_t elem_offset,
                         size_t elem_count) {
    _tex_coord_buffer->set_data(tex_coord, elem_offset, elem_count);
}

size_t Mesh::triangle_count() const {
    return _triangle_count;
}

void Mesh::set_triangle_count(size_t count) {
    _triangle_count = count;
}

void Mesh::set_indices(glm::u32vec3 *indices,
                       size_t elem_offset,
                       size_t elem_count) {
    _index_buffer->set_data(indices, elem_offset, elem_count);
}

std::shared_ptr<Mesh> upcast(const std::shared_ptr<IMesh> &mesh) {
    return std::reinterpret_pointer_cast<Mesh>(mesh);
}
} // namespace ars::render::vk
