#pragma once

#include "../IEffect.h"
#include "Texture.h"

namespace ars::render::vk {
class Context;
struct DescriptorEncoder;
class RenderGraphPassBuilder;
class ComputePipeline;
class Buffer;
struct RenderGraph;
class View;

class ImageBasedLighting {
  public:
    explicit ImageBasedLighting(Context *context);
    ~ImageBasedLighting();

    // Specify another pipeline when you need special panorama uv mapping
    void capture_to_cube_map(CommandBuffer *cmd,
                             const Handle<Texture> &panorama_tex,
                             const Handle<Texture> &env_cube_map,
                             ComputePipeline *pipeline = nullptr);
    void prefilter_irradiance(CommandBuffer *cmd,
                              const Handle<Texture> &env_cube_map,
                              const Handle<Texture> &irradiance_cube_map,
                              int sample_count,
                              float bias);

  private:
    Context *_context = nullptr;
    std::unique_ptr<ComputePipeline> _prefilter_env_pipeline{};
    std::unique_ptr<ComputePipeline> _capture_panorama_pipeline{};
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
    // Specify another pipeline when you need special panorama uv mapping.
    // When a pipeline other than the one used for the previous capture is
    // specified, this method will consider cache as outdated and perform
    // update.
    void update_cache(RenderGraph &rg,
                      ComputePipeline *capture_env_pipeline = nullptr);

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
    bool cache_dirty(ComputePipeline *pipeline);
    void ensure_irradiance_cube_map();

    Context *_context = nullptr;
    // Trivially average filtered environment map for calculate prefiltered
    // irradiance
    Handle<Texture> _tmp_env_map{};
    Handle<Texture> _irradiance_cube_map{};
    uint32_t _irradiance_cube_map_size = 64;
    Handle<Texture> _panorama_texture{};
    bool _dirty = false;
    // Only used as a mark, may be a dangling pointer, use void* to avoid any
    // call to underling data
    void *_last_capture_env_pipeline = nullptr;

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
    virtual void update([[maybe_unused]] View *view, RenderGraph &rg);
    virtual void render_background([[maybe_unused]] View *view,
                                   RenderGraph &rg);

  protected:
    // Specify another pipeline when you need special panorama uv mapping
    void
    render_background(View *view, RenderGraph &rg, ComputePipeline *pipeline);

  private:
    std::unique_ptr<SkyData> _data = nullptr;
    std::unique_ptr<ComputePipeline> _shade_background_panorama_pipeline{};
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

struct AtmosphereSettings {
    float bottom_radius;
    float top_radius;
    float mie_scattering;
    float mie_absorption;
    glm::vec3 rayleigh_scattering;
    float rayleigh_altitude;
    glm::vec3 ozone_absorption;
    float mie_altitude;
    float ozone_altitude;
    float ozone_thickness;
    float ground_albedo;
    float mie_g;
    float world_center_altitude;
};

class PhysicalSky : public IPhysicalSky, public SkyBase {
  public:
    explicit PhysicalSky(Context *context);
    ~PhysicalSky() override;

    ARS_SKY_BASE_FORWARD_COLOR_METHOD();

    void update(View *view, RenderGraph &rg) override;
    void render_background(View *view, RenderGraph &rg) override;
    Handle<Buffer> atmosphere_settings_buffer();
    Handle<Texture> transmittance_lut();

    ARS_OVERRIDE_SIMPLE_ABSTRACT_SETTER_GETTER(
        float, bottom_raidus_km, _atmosphere_settings.bottom_radius);
    ARS_OVERRIDE_SIMPLE_ABSTRACT_SETTER_GETTER(float,
                                               top_altitude_km,
                                               _top_altitude_km);
    ARS_OVERRIDE_SIMPLE_ABSTRACT_SETTER_GETTER(
        float, mie_scattering, _atmosphere_settings.mie_scattering);
    ARS_OVERRIDE_SIMPLE_ABSTRACT_SETTER_GETTER(
        float, mie_absorption, _atmosphere_settings.mie_absorption);
    ARS_OVERRIDE_SIMPLE_ABSTRACT_SETTER_GETTER(float,
                                               rayleigh_scattering_strength,
                                               _rayleigh_scattering_strength);
    ARS_OVERRIDE_SIMPLE_ABSTRACT_SETTER_GETTER(glm::vec3,
                                               rayleigh_scattering,
                                               _rayleigh_scattering);
    ARS_OVERRIDE_SIMPLE_ABSTRACT_SETTER_GETTER(
        float, rayleigh_altitude, _atmosphere_settings.rayleigh_altitude);
    ARS_OVERRIDE_SIMPLE_ABSTRACT_SETTER_GETTER(float,
                                               ozone_absorption_strength,
                                               _ozone_absorption_strength);
    ARS_OVERRIDE_SIMPLE_ABSTRACT_SETTER_GETTER(glm::vec3,
                                               ozone_absorption,
                                               _ozone_absorption);
    ARS_OVERRIDE_SIMPLE_ABSTRACT_SETTER_GETTER(
        float, mie_altitude, _atmosphere_settings.mie_altitude);
    ARS_OVERRIDE_SIMPLE_ABSTRACT_SETTER_GETTER(
        float, ozone_altitude, _atmosphere_settings.ozone_altitude);
    ARS_OVERRIDE_SIMPLE_ABSTRACT_SETTER_GETTER(
        float, ozone_thickness, _atmosphere_settings.ozone_thickness);
    ARS_OVERRIDE_SIMPLE_ABSTRACT_SETTER_GETTER(
        float, ground_albedo, _atmosphere_settings.ground_albedo);
    ARS_OVERRIDE_SIMPLE_ABSTRACT_SETTER_GETTER(float,
                                               mie_g,
                                               _atmosphere_settings.mie_g);
    ARS_OVERRIDE_SIMPLE_ABSTRACT_SETTER_GETTER(
        float,
        world_center_altitude_km,
        _atmosphere_settings.world_center_altitude);

  private:
    void init_pipelines();
    void init_atmosphere_settings_buffer();
    void init_textures();
    void update_atmosphere_settings_buffer();
    void update_transmittance_lut(RenderGraph &rg);
    void update_multi_scattering_lut(RenderGraph &rg);
    void update_sky_view_lut(View *view, RenderGraph &rg);
    void ray_march_sky_view(View *view, RenderGraph &rg, bool background);
    void update_aerial_perspective_lut(View *view, RenderGraph &rg);
    void ray_march_background(View *view, RenderGraph &rg);
    bool should_ray_march_background(View *view) const;

    Context *_context = nullptr;
    std::unique_ptr<ComputePipeline> _sky_view_lut_pipeline{};
    std::unique_ptr<ComputePipeline> _transmittance_lut_pipeline{};
    std::unique_ptr<ComputePipeline> _multi_scattering_lut_pipeline{};
    std::unique_ptr<ComputePipeline> _aerial_perspective_lut_pipeline{};
    std::unique_ptr<ComputePipeline> _shade_sky_view_lut_background_pipeline{};
    std::unique_ptr<ComputePipeline> _shade_ray_march_background_pipeline{};
    std::unique_ptr<ComputePipeline> _capture_sky_view_to_cube_map_pipeline{};
    std::unique_ptr<ComputePipeline> _apply_aerial_perspective_pipeline{};

    AtmosphereSettings _atmosphere_settings{};
    float _top_altitude_km{};
    float _rayleigh_scattering_strength = {};
    glm::vec3 _rayleigh_scattering = {};
    float _ozone_absorption_strength = {};
    glm::vec3 _ozone_absorption = {};

    Handle<Buffer> _atmosphere_settings_buffer{};
    Handle<Texture> _transmittance_lut{};
    Handle<Texture> _multi_scattering_lut{};
    Handle<Texture> _aerial_perspective_lut{};
};

std::shared_ptr<PhysicalSky> upcast(const std::shared_ptr<IPhysicalSky> &sky);

std::shared_ptr<SkyBase> upcast(const std::shared_ptr<ISky> &sky);
} // namespace ars::render::vk