#include "Effect.h"

namespace ars::render::vk {
IScreenSpaceReflectionEffect *Effect::screen_space_reflection() {
    return _ssr_effect.get();
}

Effect::Effect() {
    _ssr_effect = std::make_unique<ScreenSpaceReflectionEffect>();
}

bool ScreenSpaceReflectionEffect::enabled() {
    return _enabled;
}

void ScreenSpaceReflectionEffect::set_enabled(bool enabled) {
    _enabled = enabled;
}

float ScreenSpaceReflectionEffect::sampling_bias() {
    return _sampling_bias;
}

void ScreenSpaceReflectionEffect::set_sampling_bias(float bias) {
    _sampling_bias = bias;
}
} // namespace ars::render::vk