#pragma once

#include "../IMesh.h"

namespace ars::render::vk {
class Mesh : public IMesh {
  public:
    explicit Mesh(const MeshInfo &info);

    void set_position(glm::vec3 *positions,
                      size_t start_index,
                      size_t count) override;

    void
    set_normal(glm::vec3 *normals, size_t start_index, size_t count) override;

    void
    set_tangent(glm::vec4 *tangents, size_t start_index, size_t count) override;

    void set_tex_coord(glm::vec2 *tex_coord,
                       size_t start_index,
                       size_t count) override;

    size_t index_count() const override;

    void set_index_count(size_t count) override;
};
} // namespace ars::render::vk
