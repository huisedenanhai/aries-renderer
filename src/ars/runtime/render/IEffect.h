#pragma once

namespace ars::render {
class IScreenSpaceReflectionEffect {
  public:
    virtual bool enabled() = 0;
    virtual void set_enabled(bool enabled) = 0;
};

class IEffect {
  public:
    virtual IScreenSpaceReflectionEffect *screen_space_reflection() = 0;
};
} // namespace ars::render