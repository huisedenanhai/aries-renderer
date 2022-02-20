#pragma once

#include "../IEffect.h"
#include "Texture.h"

namespace ars::render::vk {
class Context;
class ComputePipeline;
class Buffer;
struct RenderGraph;

class ImageBasedLighting {
  public:
    explicit ImageBasedLighting(Context *context);
    ~ImageBasedLighting();

    void capture_panorama_to_cube_map(CommandBuffer *cmd,
                                      const Handle<Texture> &panorama_tex,
                                      const Handle<Texture> &env_cube_map);
    void prefilter_irradiance(CommandBuffer *cmd,
                              const Handle<Texture> &env_cube_map,
                              const Handle<Texture> &irradiance_cube_map,
                              int sample_count,
                              float bias);

  private:
    Context *_context = nullptr;
    std::unique_ptr<ComputePipeline> _prefilter_env_pipeline{};
    std::unique_ptr<ComputePipeline> _capture_cube_map_pipeline{};
};

class SkyData {
  public:
    explicit SkyData(Context *context);
    ~SkyData();

    Handle<Texture> panorama();
    void set_panorama(const Handle<Texture> &hdr);
    uint32_t irradiance_cube_map_size() const;
    void set_irradiance_cube_map_size(uint32_t resolution);
    Handle<Texture> irradiance_cube_map();

    void mark_dirty(bool dirty = true);
    void update_cache(RenderGraph &rg);
    bool cache_dirty();

    glm::vec3 color() const;
    void set_color(glm::vec3 color);
    float strength() const;
    void set_strength(float strength);
    glm::vec3 radiance() const;
    float prefilter_bias() const;
    void set_prefilter_bias(float bias);
    int32_t prefilter_sample_count() const;
    void set_prefilter_sample_count(int32_t sample_count);

  private:
    void ensure_irradiance_cube_map();

    Context *_context = nullptr;
    // Trivially average filtered environment map for calculate prefiltered
    // irradiance
    Handle<Texture> _tmp_env_map{};
    Handle<Texture> _irradiance_cube_map{};
    uint32_t _irradiance_cube_map_size = 64;
    Handle<Texture> _panorama_texture{};
    bool _dirty = false;

    float _prefilter_bias = 3.0f;
    int32_t _prefilter_sample_count = 256;

    glm::vec3 _color{1.0f, 1.0f, 1.0f};
    float _strength = 1.0f;
};

#define ARS_SKY_BASE_FORWARD_COLOR_METHOD()                                    \
    glm::vec3 color() override {                                               \
        return data()->color();                                                \
    }                                                                          \
    void set_color(glm::vec3 color) override {                                 \
        data()->set_color(color);                                              \
    }                                                                          \
    float strength() override {                                                \
        return data()->strength();                                             \
    }                                                                          \
    void set_strength(float strength) override {                               \
        data()->set_strength(strength);                                        \
    }

class SkyBase {
  public:
    explicit SkyBase(Context *context);
    virtual ~SkyBase() = default;
    SkyData *data();
    virtual void render(RenderGraph &rg);

  private:
    std::unique_ptr<SkyData> _data = nullptr;
};

class PanoramaSky : public IPanoramaSky, public SkyBase {
  public:
    explicit PanoramaSky(Context *context);
    ~PanoramaSky() override;

    ARS_SKY_BASE_FORWARD_COLOR_METHOD();

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
    ~PhysicalSky() override;

    ARS_SKY_BASE_FORWARD_COLOR_METHOD();

    void render(RenderGraph &rg) override;

  private:
    void init_pipelines();
    void init_atmosphere_settings_buffer();
    void init_textures();
    void update_transmittance_lut(RenderGraph &rg);
    void update_sky_view(RenderGraph &rg);

    Context *_context = nullptr;
    std::unique_ptr<ComputePipeline> _physical_panorama_pipeline{};
    std::unique_ptr<ComputePipeline> _transmittance_lut_pipeline{};
    Handle<Buffer> _atmosphere_settings_buffer{};
    Handle<Texture> _transmittance_lut{};
};

std::shared_ptr<PhysicalSky> upcast(const std::shared_ptr<IPhysicalSky> &sky);

std::shared_ptr<SkyBase> upcast(const std::shared_ptr<ISky> &sky);
} // namespace ars::render::vk