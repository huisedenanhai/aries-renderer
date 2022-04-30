#pragma once

#include <ars/runtime/core/Reflect.h>
#include <ars/runtime/core/misc/Macro.h>
#include <glm/vec3.hpp>

namespace ars::render {
class ITexture;

class IScreenSpaceReflectionEffect {
  public:
    virtual bool enabled() = 0;
    virtual void set_enabled(bool enabled) = 0;

    // Value in [0, 1]
    // Adjust this parameter to reduce fireflies
    virtual float sampling_bias() = 0;
    virtual void set_sampling_bias(float bias) = 0;

    virtual float border_fade() = 0;
    virtual void set_border_fade(float fade) = 0;

    virtual float thickness() = 0;
    virtual void set_thickness(float thickness) = 0;

    static void register_type();
};

class ISky {
    RTTR_ENABLE();

  public:
    virtual ~ISky() = default;

    virtual glm::vec3 color() = 0;
    virtual void set_color(glm::vec3 radiance) = 0;

    virtual float strength() = 0;
    virtual void set_strength(float strength) = 0;

    static void register_type();
};

class IPanoramaSky : public ISky {
    RTTR_DERIVE(ISky);

  public:
    virtual std::shared_ptr<ITexture> panorama() = 0;
    virtual void set_panorama(std::shared_ptr<ITexture> hdr) = 0;

    // Alloc a new cube map with given size for IBL. One should call
    // update_irradiance_cube_map() to update its content.
    virtual uint32_t irradiance_cube_map_size() = 0;
    virtual void set_irradiance_cube_map_size(uint32_t resolution) = 0;

    static void register_type();
};

class IPhysicalSky : public ISky {
    RTTR_DERIVE(ISky);

  public:
    ARS_DECLARE_SIMPLE_ABSTRACT_SETTER_GETTER(float, bottom_raidus_km);
    ARS_DECLARE_SIMPLE_ABSTRACT_SETTER_GETTER(float, top_altitude_km);
    ARS_DECLARE_SIMPLE_ABSTRACT_SETTER_GETTER(float, mie_scattering);
    ARS_DECLARE_SIMPLE_ABSTRACT_SETTER_GETTER(float, mie_absorption);
    ARS_DECLARE_SIMPLE_ABSTRACT_SETTER_GETTER(float,
                                              rayleigh_scattering_strength);
    ARS_DECLARE_SIMPLE_ABSTRACT_SETTER_GETTER(glm::vec3, rayleigh_scattering);
    ARS_DECLARE_SIMPLE_ABSTRACT_SETTER_GETTER(float, rayleigh_altitude);
    ARS_DECLARE_SIMPLE_ABSTRACT_SETTER_GETTER(float, ozone_absorption_strength);
    ARS_DECLARE_SIMPLE_ABSTRACT_SETTER_GETTER(glm::vec3, ozone_absorption);
    ARS_DECLARE_SIMPLE_ABSTRACT_SETTER_GETTER(float, mie_altitude);
    ARS_DECLARE_SIMPLE_ABSTRACT_SETTER_GETTER(float, ozone_altitude);
    ARS_DECLARE_SIMPLE_ABSTRACT_SETTER_GETTER(float, ozone_thickness);
    ARS_DECLARE_SIMPLE_ABSTRACT_SETTER_GETTER(float, ground_albedo);
    ARS_DECLARE_SIMPLE_ABSTRACT_SETTER_GETTER(float, mie_g);
    ARS_DECLARE_SIMPLE_ABSTRACT_SETTER_GETTER(float, world_center_altitude_km);

    static void register_type();
};

enum class BackgroundMode { Color, Sky };

class IBackground {
  public:
    virtual BackgroundMode mode() = 0;
    virtual void set_mode(BackgroundMode mode) = 0;

    virtual glm::vec3 color() = 0;
    virtual void set_color(glm::vec3 radiance) = 0;

    virtual float strength() = 0;
    virtual void set_strength(float strength) = 0;

    virtual std::shared_ptr<ISky> sky() = 0;
    virtual void set_sky(std::shared_ptr<ISky> sky) = 0;

    static void register_type();
};

class IEffect {
  public:
    virtual IScreenSpaceReflectionEffect *screen_space_reflection() = 0;
    virtual IScreenSpaceReflectionEffect *
    screen_space_global_illumination() = 0;
    virtual IBackground *background() = 0;

    static void register_type();
};
} // namespace ars::render