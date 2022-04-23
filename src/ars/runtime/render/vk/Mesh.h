#pragma once

#include "../IMesh.h"
#include "Vulkan.h"

namespace ars::render::vk {
class Context;
class Buffer;

class Mesh : public IMesh {
  public:
    Mesh(Context *context, const MeshInfo &info);

    void set_position(const glm::vec3 *positions,
                      size_t elem_offset,
                      size_t elem_count) override;

    void set_normal(const glm::vec3 *normals,
                    size_t elem_offset,
                    size_t elem_count) override;

    void set_tangent(const glm::vec4 *tangents,
                     size_t elem_offset,
                     size_t elem_count) override;

    void set_tex_coord(const glm::vec2 *tex_coord,
                       size_t elem_offset,
                       size_t elem_count) override;

    void set_joint(const glm::uvec4 *joints,
                   size_t elem_offset,
                   size_t elem_count) override;

    void set_weight(const glm::vec4 *weights,
                    size_t elem_offset,
                    size_t elem_count) override;

    [[nodiscard]] size_t triangle_count() const override;

    void set_triangle_count(size_t count) override;

    void set_indices(const glm::u32vec3 *indices,
                     size_t elem_offset,
                     size_t elem_count) override;
    math::AABB<float> aabb() override;
    void set_aabb(const math::AABB<float> &aabb) override;

    [[nodiscard]] Handle<Buffer> position_buffer() const;
    [[nodiscard]] Handle<Buffer> normal_buffer() const;
    [[nodiscard]] Handle<Buffer> tangent_buffer() const;
    [[nodiscard]] Handle<Buffer> tex_coord_buffer() const;
    [[nodiscard]] Handle<Buffer> index_buffer() const;
    [[nodiscard]] Handle<Buffer> joint_buffer() const;
    [[nodiscard]] Handle<Buffer> weight_buffer() const;

  private:
    Context *_context = nullptr;

    Handle<Buffer> _position_buffer{};
    Handle<Buffer> _normal_buffer{};
    Handle<Buffer> _tangent_buffer{};
    Handle<Buffer> _tex_coord_buffer{};
    Handle<Buffer> _joint_buffer{};
    Handle<Buffer> _weight_buffer{};

    Handle<Buffer> _index_buffer{};
    size_t _triangle_count = 0;
    math::AABB<float> _aabb{};
};

std::shared_ptr<Mesh> upcast(const std::shared_ptr<IMesh> &mesh);

class Skeleton : public ISkeleton {
  public:
    Skeleton(Context *context, const SkeletonInfo &info);

    void set_joints(const glm::mat4 *joints,
                    size_t joint_offset,
                    size_t joint_count) override;

    [[nodiscard]] Handle<Buffer> joint_buffer() const;

  private:
    Context *_context = nullptr;
    Handle<Buffer> _joint_buffer{};
};

std::shared_ptr<Skeleton> upcast(const std::shared_ptr<ISkeleton> &skeleton);
} // namespace ars::render::vk
