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

  private:
    bool _enabled = false;
    float _sampling_bias = 0.1f;
};

class Effect : public IEffect {
  public:
    Effect();

    IScreenSpaceReflectionEffect *screen_space_reflection() override;

  private:
    std::unique_ptr<ScreenSpaceReflectionEffect> _ssr_effect{};
};
} // namespace ars::render::vk