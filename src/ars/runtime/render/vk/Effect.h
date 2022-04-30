#pragma once

#include "../IEffect.h"
#include <memory>

namespace ars::render::vk {
class View;
class SkyBase;
class PanoramaSky;

class ScreenSpaceReflectionEffect : public IScreenSpaceReflectionEffect {
  public:
    bool enabled() override;
    void set_enabled(bool enabled) override;
    float sampling_bias() override;
    void set_sampling_bias(float bias) override;
    float border_fade() override;
    void set_border_fade(float fade) override;
    float thickness() override;
    void set_thickness(float thickness) override;

  private:
    bool _enabled = false;
    float _sampling_bias = 0.1f;
    float _thickness = 0.1f;
    float _border_fade = 0.01f;
};

class Background : public IBackground {
  public:
    explicit Background(View *view);

    BackgroundMode mode() override;
    void set_mode(BackgroundMode mode) override;
    glm::vec3 color() override;
    void set_color(glm::vec3 radiance) override;
    float strength() override;
    void set_strength(float strength) override;
    std::shared_ptr<ISky> sky() override;
    void set_sky(std::shared_ptr<ISky> sky) override;

    // Not null
    SkyBase *sky_vk();
    glm::vec3 radiance();

  private:
    View *_view{};
    BackgroundMode _mode = BackgroundMode::Sky;
    glm::vec3 _color = glm::vec3(0.1f);
    float _strength = 1.0f;
    std::shared_ptr<ISky> _sky{};
    std::shared_ptr<PanoramaSky> _default_sky{};
};

class Effect : public IEffect {
  public:
    explicit Effect(View *view);

    IScreenSpaceReflectionEffect *screen_space_reflection() override;
    IScreenSpaceReflectionEffect *screen_space_global_illumination() override;
    ScreenSpaceReflectionEffect *screen_space_reflection_vk();
    IBackground *background() override;
    Background *background_vk();

  private:
    std::unique_ptr<ScreenSpaceReflectionEffect> _ssr_effect{};
    std::unique_ptr<ScreenSpaceReflectionEffect> _ssgi_effect{};
    std::unique_ptr<Background> _background{};
};
} // namespace ars::render::vk