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

float ScreenSpaceReflectionEffect::border_fade() {
    return _border_fade;
}

void ScreenSpaceReflectionEffect::set_border_fade(float fade) {
    _border_fade = fade;
}

float ScreenSpaceReflectionEffect::thickness() {
    return _thickness;
}

void ScreenSpaceReflectionEffect::set_thickness(float thickness) {
    _thickness = thickness;
}
} // namespace ars::render::vk