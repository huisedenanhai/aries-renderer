#pragma once

namespace ars::render {
class IScreenSpaceReflectionEffect {
  public:
    virtual bool enabled() = 0;
    virtual void set_enabled(bool enabled) = 0;

    // Value in [0, 1]
    // Adjust this parameter to reduce fireflies
    virtual float sampling_bias() = 0;
    virtual void set_sampling_bias(float bias) = 0;

    static void register_type();
};

class IEffect {
  public:
    virtual IScreenSpaceReflectionEffect *screen_space_reflection() = 0;

    static void register_type();
};
} // namespace ars::render