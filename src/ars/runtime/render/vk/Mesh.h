#pragma once

#include "../IMesh.h"
#include "Vulkan.h"

namespace ars::render::vk {
class Context;
class Buffer;

class Mesh : public IMesh {
  public:
    Mesh(Context *context, const MeshInfo &info);

    void set_position(glm::vec3 *positions,
                      size_t elem_offset,
                      size_t elem_count) override;

    void set_normal(glm::vec3 *normals,
                    size_t elem_offset,
                    size_t elem_count) override;

    void set_tangent(glm::vec4 *tangents,
                     size_t elem_offset,
                     size_t elem_count) override;

    void set_tex_coord(glm::vec2 *tex_coord,
                       size_t elem_offset,
                       size_t elem_count) override;

    [[nodiscard]] size_t triangle_count() const override;

    void set_triangle_count(size_t count) override;

    void set_indices(glm::u32vec3 *indices,
                     size_t elem_offset,
                     size_t elem_count) override;

    Handle<Buffer> position_buffer() const;
    Handle<Buffer> normal_buffer() const;
    Handle<Buffer> tangent_buffer() const;
    Handle<Buffer> tex_coord_buffer() const;
    Handle<Buffer> index_buffer() const;

  private:
    Context *_context = nullptr;

    Handle<Buffer> _position_buffer{};
    Handle<Buffer> _normal_buffer{};
    Handle<Buffer> _tangent_buffer{};
    Handle<Buffer> _tex_coord_buffer{};

    Handle<Buffer> _index_buffer{};
    size_t _triangle_count = 0;
};

std::shared_ptr<Mesh> upcast(const std::shared_ptr<IMesh> &mesh);
} // namespace ars::render::vk
