#pragma once

#include "../IEffect.h"
#include <memory>

namespace ars::render::vk {
class ScreenSpaceReflectionEffect : public IScreenSpaceReflectionEffect {
  public:
    bool enabled() override;
    void set_enabled(bool enabled) override;

  private:
    bool _enabled = false;
};

class Effect : public IEffect {
  public:
    Effect();

    IScreenSpaceReflectionEffect *screen_space_reflection() override;

  private:
    std::unique_ptr<ScreenSpaceReflectionEffect> _ssr_effect{};
};
} // namespace ars::render::vk