#pragma once

#include <ars/runtime/core/Reflect.h>
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
    virtual IBackground *background() = 0;

    static void register_type();
};
} // namespace ars::render