#include "Mesh.h"
#include "Buffer.h"
#include "Context.h"
#include "features/RayTracing.h"
#include <ars/runtime/core/Log.h>

namespace ars::render::vk {
Mesh::Mesh(Context *context, const MeshInfo &info)
    : IMesh(info), _context(context) {
    auto vert_heap = _context->heap(NamedHeap_Vertices);
    _position_buffer =
        vert_heap->alloc_owned(info.vertex_capacity * sizeof(glm::vec3));
    _normal_buffer =
        vert_heap->alloc_owned(info.vertex_capacity * sizeof(glm::vec3));
    _tangent_buffer =
        vert_heap->alloc_owned(info.vertex_capacity * sizeof(glm::vec4));
    _tex_coord_buffer =
        vert_heap->alloc_owned(info.vertex_capacity * sizeof(glm::vec2));

    if (_info.skinned) {
        _joint_buffer =
            vert_heap->alloc_owned(info.vertex_capacity * sizeof(glm::uvec4));
        _weight_buffer =
            vert_heap->alloc_owned(info.vertex_capacity * sizeof(glm::vec4));
    }

    _index_buffer =
        _context->heap(NamedHeap_Indices)
            ->alloc_owned(_info.triangle_capacity * sizeof(glm::u32vec3));
}

void Mesh::set_position(const glm::vec3 *positions,
                        size_t elem_offset,
                        size_t elem_count) {
    position_buffer().set_data_array(positions, elem_offset, elem_count);
}

void Mesh::set_normal(const glm::vec3 *normals,
                      size_t elem_offset,
                      size_t elem_count) {
    normal_buffer().set_data_array(normals, elem_offset, elem_count);
}

void Mesh::set_tangent(const glm::vec4 *tangents,
                       size_t elem_offset,
                       size_t elem_count) {
    tangent_buffer().set_data_array(tangents, elem_offset, elem_count);
}

void Mesh::set_tex_coord(const glm::vec2 *tex_coord,
                         size_t elem_offset,
                         size_t elem_count) {
    tex_coord_buffer().set_data_array(tex_coord, elem_offset, elem_count);
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
    index_buffer().set_data_array(indices, elem_offset, elem_count);
}

HeapRange Mesh::position_buffer() const {
    return _position_buffer->slice();
}

HeapRange Mesh::normal_buffer() const {
    return _normal_buffer->slice();
}

HeapRange Mesh::tangent_buffer() const {
    return _tangent_buffer->slice();
}

HeapRange Mesh::tex_coord_buffer() const {
    return _tex_coord_buffer->slice();
}

HeapRange Mesh::index_buffer() const {
    return _index_buffer->slice();
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
    joint_buffer().set_data_array(joints, elem_offset, elem_count);
}

void Mesh::set_weight(const glm::vec4 *weights,
                      size_t elem_offset,
                      size_t elem_count) {
    weight_buffer().set_data_array(weights, elem_offset, elem_count);
}

HeapRange Mesh::joint_buffer() const {
    return _joint_buffer->slice();
}

HeapRange Mesh::weight_buffer() const {
    return _weight_buffer->slice();
}

void Mesh::update_acceleration_structure() {
    auto &acc_feature = _context->info().acceleration_structure_features;
    if (acc_feature.accelerationStructure == VK_FALSE) {
        return;
    }

    _acceleration_structure = AccelerationStructure::create(this);
}

Context *Mesh::context() const {
    return _context;
}

Handle<AccelerationStructure> Mesh::acceleration_structure() const {
    return _acceleration_structure;
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
