#pragma once

#include <glm/vec3.hpp>
#include <memory>

namespace ars::render {
class ITexture;

class IEnvironment {
  public:
    virtual glm::vec3 radiance() = 0;
    virtual void set_radiance(const glm::vec3 &radiance) = 0;

    virtual std::shared_ptr<ITexture> hdr_texture() = 0;
    virtual void set_hdr_texture(const std::shared_ptr<ITexture> &hdr) = 0;

    // The default white cube map texture will be returned if no cube map is
    // alloc or set.
    virtual std::shared_ptr<ITexture> cube_map() = 0;
    // Set a precomputed cube map for IBL.
    // When call update_cube_map, the content of the cube map will be modified.
    virtual void set_cube_map(const std::shared_ptr<ITexture> &cube_map) = 0;
    // Alloc a new cube map with given size for IBL. One should call
    // update_cube_map() to update its content.
    virtual void alloc_cube_map(uint32_t resolution) = 0;
    // Cube map caches filtered irradiance.
    // This method should be called after hdr texture or cube map changes to
    // make filtered environment irradiance correct.
    virtual void update_cube_map() = 0;
};
} // namespace ars::render