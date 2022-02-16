#pragma once

#include "../IEffect.h"
#include "Texture.h"

namespace ars::render::vk {
class Context;
class ComputePipeline;
struct RenderGraph;

class SkyData {
  public:
    explicit SkyData(Context *context);
    ~SkyData();

    Handle<Texture> panorama();
    void set_panorama(const Handle<Texture> &hdr);
    uint32_t irradiance_cube_map_size() const;
    void set_irradiance_cube_map_size(uint32_t resolution);
    Handle<Texture> irradiance_cube_map();
    void update_cache(RenderGraph &rg);
    bool cache_dirty();

  private:
    void ensure_irradiance_cube_map();
    void capture_env_cube_map(CommandBuffer *cmd,
                              const Handle<Texture> &panorama_tex,
                              const Handle<Texture> &env_map);
    void prefilter_irradiance(CommandBuffer *cmd,
                              const Handle<Texture> &env_map,
                              const Handle<Texture> &irradiance_map);

    Context *_context = nullptr;
    std::unique_ptr<ComputePipeline> _prefilter_env_pipeline{};
    std::unique_ptr<ComputePipeline> _capture_cube_map_pipeline{};
    // Trivially average filtered environment map for calculate prefiltered
    // irradiance
    Handle<Texture> _tmp_env_map{};
    Handle<Texture> _irradiance_cube_map{};
    uint32_t _irradiance_cube_map_size = 64;
    Handle<Texture> _panorama_texture{};
    bool _panorama_dirty = false;
};

class SkyBase {
  public:
    explicit SkyBase(Context *context);
    virtual SkyData *data();

  private:
    std::unique_ptr<SkyData> _data = nullptr;
};

class PanoramaSky : public IPanoramaSky, public SkyBase {
  public:
    explicit PanoramaSky(Context *context);
    ~PanoramaSky();
    std::shared_ptr<ITexture> panorama() override;
    void set_panorama(std::shared_ptr<ITexture> hdr) override;
    uint32_t irradiance_cube_map_size() override;
    void set_irradiance_cube_map_size(uint32_t resolution) override;

  private:
    Context *_context = nullptr;
    std::shared_ptr<ITexture> _panorama{};
};

std::shared_ptr<PanoramaSky> upcast(const std::shared_ptr<IPanoramaSky> &sky);

class PhysicalSky : public IPhysicalSky, public SkyBase {
  public:
    explicit PhysicalSky(Context *context);

  private:
    Context *_context = nullptr;
};

std::shared_ptr<PhysicalSky> upcast(const std::shared_ptr<IPhysicalSky> &sky);

std::shared_ptr<SkyBase> upcast(const std::shared_ptr<ISky> &sky);
} // namespace ars::render::vk