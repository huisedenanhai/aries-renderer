#pragma once

#include "../IEnvironment.h"
#include "Texture.h"

namespace ars::render::vk {
class Context;

class Environment : public IEnvironment {
  public:
    explicit Environment(Context *context);
    glm::vec3 radiance() override;
    void set_radiance(const glm::vec3 &radiance) override;

    std::shared_ptr<ITexture> hdr_texture() override;
    void set_hdr_texture(const std::shared_ptr<ITexture> &hdr) override;
    Handle<Texture> hdr_texture_vk();

    std::shared_ptr<ITexture> cube_map() override;
    void set_cube_map_resolution(uint32_t resolution) override;
    Handle<Texture> cube_map_vk();

    void update_cache() override;

  private:
    Context *_context = nullptr;
    glm::vec3 _radiance = glm::vec3(0.1f);
    uint32_t _cube_map_size = 1;
    std::shared_ptr<ITexture> _cube_map{};
    std::shared_ptr<ITexture> _hdr_texture{};
};
} // namespace ars::render::vk