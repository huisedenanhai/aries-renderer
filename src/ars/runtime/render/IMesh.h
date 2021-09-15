#pragma once

#include <glm/glm.hpp>
#include <memory>

namespace ars::render {
struct MeshInfo {
    size_t vertex_capacity = 0;
    size_t index_capacity = 0;
};

class IMesh {
  public:
    explicit IMesh(const MeshInfo &info);

    // Capacity can not be changed after initialization.
    size_t vertex_capacity() const;
    size_t index_capacity() const;

    // Values exceed capacity will be ignored.
    virtual void
    set_position(glm::vec3 *positions, size_t start_index, size_t count) = 0;
    virtual void
    set_normal(glm::vec3 *normals, size_t start_index, size_t count) = 0;
    virtual void
    set_tangent(glm::vec4 *tangents, size_t start_index, size_t count) = 0;
    virtual void
    set_tex_coord(glm::vec2 *tex_coord, size_t start_index, size_t count) = 0;

    virtual size_t index_count() const = 0;
    virtual void set_index_count(size_t count) = 0;

    virtual ~IMesh() = default;

  protected:
    MeshInfo _info{};
};
} // namespace ars::render
