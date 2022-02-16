#include "Effect.h"
#include "Context.h"
#include "Sky.h"
#include "View.h"

namespace ars::render::vk {
IScreenSpaceReflectionEffect *Effect::screen_space_reflection() {
    return screen_space_reflection_vk();
}

Effect::Effect(View *view) {
    _ssr_effect = std::make_unique<ScreenSpaceReflectionEffect>();
    _background = std::make_unique<Background>(view);
}

IBackground *Effect::background() {
    return background_vk();
}

ScreenSpaceReflectionEffect *Effect::screen_space_reflection_vk() {
    return _ssr_effect.get();
}

Background *Effect::background_vk() {
    return _background.get();
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

BackgroundMode Background::mode() {
    return _mode;
}

void Background::set_mode(BackgroundMode mode) {
    _mode = mode;
}

glm::vec3 Background::color() {
    return _color;
}

void Background::set_color(glm::vec3 radiance) {
    _color = radiance;
}

float Background::strength() {
    return _strength;
}

void Background::set_strength(float strength) {
    _strength = strength;
}

std::shared_ptr<ISky> Background::sky() {
    return _sky;
}

void Background::set_sky(std::shared_ptr<ISky> sky) {
    _sky = sky;
}

SkyData *Background::sky_data() {
    if (_sky == nullptr) {
        return _default_sky->data();
    }
    return upcast(_sky)->data();
}

Background::Background(View *view) : _view(view) {
    _default_sky = upcast(_view->context()->create_panorama_sky());
}

glm::vec3 Background::radiance() {
    return color() * strength();
}
} // namespace ars::render::vk