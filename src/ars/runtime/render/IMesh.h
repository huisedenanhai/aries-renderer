#pragma once

#include "IContext.h"
#include <ars/runtime/core/Res.h>
#include <ars/runtime/core/math/AABB.h>
#include <glm/glm.hpp>
#include <memory>

namespace ars::render {
struct MeshInfo {
    bool skinned = false;
    size_t vertex_capacity = 0;
    size_t triangle_capacity = 0;
};

class IMesh : public IRes {
    RTTR_DERIVE(IRes);

  public:
    explicit IMesh(const MeshInfo &info);

    // Capacity can not be changed after initialization.
    [[nodiscard]] bool skinned() const;
    [[nodiscard]] size_t vertex_capacity() const;
    [[nodiscard]] size_t triangle_capacity() const;

    // Values exceed capacity will be ignored.
    virtual void set_position(const glm::vec3 *positions,
                              size_t elem_offset,
                              size_t elem_count) = 0;
    virtual void set_normal(const glm::vec3 *normals,
                            size_t elem_offset,
                            size_t elem_count) = 0;
    virtual void set_tangent(const glm::vec4 *tangents,
                             size_t elem_offset,
                             size_t elem_count) = 0;
    virtual void set_tex_coord(const glm::vec2 *tex_coord,
                               size_t elem_offset,
                               size_t elem_count) = 0;
    // Only take effects when skinned == true
    virtual void set_joint(const glm::u16vec4 *joints,
                           size_t elem_offset,
                           size_t elem_count) = 0;
    virtual void set_weight(const glm::vec4 *weights,
                            size_t elem_offset,
                            size_t elem_count) = 0;

    [[nodiscard]] virtual size_t triangle_count() const = 0;
    virtual void set_triangle_count(size_t count) = 0;

    virtual void set_indices(const glm::u32vec3 *indices,
                             size_t elem_offset,
                             size_t elem_count) = 0;

    virtual math::AABB<float> aabb() = 0;
    virtual void set_aabb(const math::AABB<float> &aabb) = 0;

    static void register_type();

  protected:
    MeshInfo _info{};
};

struct SkeletonInfo {
    uint32_t joint_count = 0;
};

class ISkeleton {
  public:
    explicit ISkeleton(const SkeletonInfo &info);

    uint32_t joint_count() const;

    // Bones are matrix directly applied for skinning
    virtual void set_joints(const glm::mat4 *joints,
                            size_t joint_offset,
                            size_t joint_count) = 0;

  protected:
    SkeletonInfo _info{};
};
} // namespace ars::render
