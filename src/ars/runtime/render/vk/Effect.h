#pragma once

#include "../IEffect.h"
#include <memory>

namespace ars::render::vk {
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

class Effect : public IEffect {
  public:
    Effect();

    IScreenSpaceReflectionEffect *screen_space_reflection() override;

  private:
    std::unique_ptr<ScreenSpaceReflectionEffect> _ssr_effect{};
};
} // namespace ars::render::vk