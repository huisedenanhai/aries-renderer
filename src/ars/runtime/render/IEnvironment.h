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

    virtual std::shared_ptr<ITexture> cube_map() = 0;
    // The cube map will be reallocated, one should call update_cache() to
    // update its content
    virtual void set_cube_map_resolution(uint32_t resolution) = 0;

    // The environment caches filtered radiance
    // This method should be called after hdr texture or cube map changes to
    // make filtered environment irradiance correct
    virtual void update_cache() = 0;
};
} // namespace ars::render