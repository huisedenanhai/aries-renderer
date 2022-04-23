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

    if (_info.skinned) {
        _joint_buffer = create_buffer(info.vertex_capacity * sizeof(glm::uvec4),
                                      vertex_buffer_usage);
        _weight_buffer = create_buffer(info.vertex_capacity * sizeof(glm::vec4),
                                       vertex_buffer_usage);
    }

    VkBufferUsageFlags index_buffer_usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    _index_buffer = create_buffer(
        _info.triangle_capacity * sizeof(glm::u32vec3), index_buffer_usage);
}

void Mesh::set_position(const glm::vec3 *positions,
                        size_t elem_offset,
                        size_t elem_count) {
    _position_buffer->set_data_array(positions, elem_offset, elem_count);
}

void Mesh::set_normal(const glm::vec3 *normals,
                      size_t elem_offset,
                      size_t elem_count) {
    _normal_buffer->set_data_array(normals, elem_offset, elem_count);
}

void Mesh::set_tangent(const glm::vec4 *tangents,
                       size_t elem_offset,
                       size_t elem_count) {
    _tangent_buffer->set_data_array(tangents, elem_offset, elem_count);
}

void Mesh::set_tex_coord(const glm::vec2 *tex_coord,
                         size_t elem_offset,
                         size_t elem_count) {
    _tex_coord_buffer->set_data_array(tex_coord, elem_offset, elem_count);
}

size_t Mesh::triangle_count() const {
    return _triangle_count;
}

void Mesh::set_triangle_count(size_t count) {
    _triangle_count = count;
}

void Mesh::set_indices(const glm::u32vec3 *indices,
                       size_t elem_offset,
                       size_t elem_count) {
    _index_buffer->set_data_array(indices, elem_offset, elem_count);
}

Handle<Buffer> Mesh::position_buffer() const {
    return _position_buffer;
}

Handle<Buffer> Mesh::normal_buffer() const {
    return _normal_buffer;
}

Handle<Buffer> Mesh::tangent_buffer() const {
    return _tangent_buffer;
}

Handle<Buffer> Mesh::tex_coord_buffer() const {
    return _tex_coord_buffer;
}

Handle<Buffer> Mesh::index_buffer() const {
    return _index_buffer;
}

math::AABB<float> Mesh::aabb() {
    return _aabb;
}

void Mesh::set_aabb(const math::AABB<float> &aabb) {
    _aabb = aabb;
}

void Mesh::set_joint(const glm::uvec4 *joints,
                     size_t elem_offset,
                     size_t elem_count) {
    if (_joint_buffer != nullptr) {
        _joint_buffer->set_data_array(joints, elem_offset, elem_count);
    }
}

void Mesh::set_weight(const glm::vec4 *weights,
                      size_t elem_offset,
                      size_t elem_count) {
    if (_weight_buffer != nullptr) {
        _weight_buffer->set_data_array(weights, elem_offset, elem_count);
    }
}

Handle<Buffer> Mesh::joint_buffer() const {
    return _joint_buffer;
}

Handle<Buffer> Mesh::weight_buffer() const {
    return _weight_buffer;
}

std::shared_ptr<Mesh> upcast(const std::shared_ptr<IMesh> &mesh) {
    return std::reinterpret_pointer_cast<Mesh>(mesh);
}

void Skin::set_joints(const glm::mat4 *joints,
                          size_t joint_offset,
                          size_t joint_count) {
    _joint_buffer->set_data_array(joints, joint_offset, joint_count);
}

Skin::Skin(Context *context, const SkinInfo &info)
    : ISkin(info), _context(context) {
    _joint_buffer =
        _context->create_buffer(info.joint_count * sizeof(glm::mat4),
                                VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                                VMA_MEMORY_USAGE_CPU_TO_GPU);
    _joint_buffer->map_once([&](void *ptr) {
        auto joints = reinterpret_cast<glm::mat4 *>(ptr);
        std::fill(
            joints, joints + _info.joint_count, glm::identity<glm::mat4>());
    });
}

Handle<Buffer> Skin::joint_buffer() const {
    return _joint_buffer;
}

std::shared_ptr<Skin> upcast(const std::shared_ptr<ISkin> &skeleton) {
    return std::dynamic_pointer_cast<Skin>(skeleton);
}
} // namespace ars::render::vk
