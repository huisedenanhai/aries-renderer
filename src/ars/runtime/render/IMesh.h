#pragma once

#include "IContext.h"
#include <ars/runtime/core/math/AABB.h>
#include <glm/glm.hpp>
#include <memory>

namespace ars::render {
struct MeshInfo {
    size_t vertex_capacity = 0;
    size_t triangle_capacity = 0;
};

class IMesh {
  public:
    explicit IMesh(const MeshInfo &info);

    // Capacity can not be changed after initialization.
    [[nodiscard]] size_t vertex_capacity() const;
    [[nodiscard]] size_t triangle_capacity() const;

    // Values exceed capacity will be ignored.
    virtual void set_position(glm::vec3 *positions,
                              size_t elem_offset,
                              size_t elem_count) = 0;
    virtual void
    set_normal(glm::vec3 *normals, size_t elem_offset, size_t elem_count) = 0;
    virtual void
    set_tangent(glm::vec4 *tangents, size_t elem_offset, size_t elem_count) = 0;
    virtual void set_tex_coord(glm::vec2 *tex_coord,
                               size_t elem_offset,
                               size_t elem_count) = 0;

    [[nodiscard]] virtual size_t triangle_count() const = 0;
    virtual void set_triangle_count(size_t count) = 0;

    virtual void set_indices(glm::u32vec3 *indices,
                             size_t elem_offset,
                             size_t elem_count) = 0;

    virtual math::AABB<float> aabb() = 0;
    virtual void set_aabb(const math::AABB<float> &aabb) = 0;

    virtual ~IMesh() = default;

  protected:
    MeshInfo _info{};
};
} // namespace ars::render
