#include "IEffect.h"
#include <rttr/registration>

namespace ars::render {
void IScreenSpaceReflectionEffect::register_type() {
    rttr::registration::class_<IScreenSpaceReflectionEffect>(
        "ars::render::IScreenSpaceReflectionEffect")
        .property("enabled",
                  &IScreenSpaceReflectionEffect::enabled,
                  &IScreenSpaceReflectionEffect::set_enabled)
        .property("sampling_bias",
                  &IScreenSpaceReflectionEffect::sampling_bias,
                  &IScreenSpaceReflectionEffect::set_sampling_bias);
}

void IEffect::register_type() {
    rttr::registration::class_<IEffect>("ars::render::IEffect")
        .property_readonly("screen_space_reflection",
                           &IEffect::screen_space_reflection);
}
} // namespace ars::render