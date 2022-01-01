#pragma once

#include "../IEnvironment.h"
#include "Texture.h"

namespace ars::render::vk {
class Context;
class ComputePipeline;

class Environment : public IEnvironment {
  public:
    explicit Environment(Context *context);
    ~Environment();

    glm::vec3 radiance() override;
    void set_radiance(const glm::vec3 &radiance) override;

    std::shared_ptr<ITexture> hdr_texture() override;
    Handle<Texture> hdr_texture_vk();
    void set_hdr_texture(const std::shared_ptr<ITexture> &hdr) override;

    std::shared_ptr<ITexture> cube_map() override;
    Handle<Texture> cube_map_vk();
    void set_cube_map(const std::shared_ptr<ITexture> &cube_map) override;
    void alloc_cube_map(uint32_t resolution) override;
    void update_cube_map() override;

  private:
    Context *_context = nullptr;
    glm::vec3 _radiance = glm::vec3(0.1f);
    std::unique_ptr<ComputePipeline> _prefilter_env_pipeline{};
    std::shared_ptr<ITexture> _cube_map{};
    std::shared_ptr<ITexture> _hdr_texture{};
};
} // namespace ars::render::vk