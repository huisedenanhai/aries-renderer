#include "IEffect.h"
#include "ITexture.h"
#include <rttr/registration>

namespace ars::render {
void IScreenSpaceReflectionEffect::register_type() {
    rttr::registration::class_<IScreenSpaceReflectionEffect>(
        "ars::render::IScreenSpaceReflectionEffect")
        .RTTR_MEMBER_PROPERTY(IScreenSpaceReflectionEffect, enabled)
        .RTTR_MEMBER_PROPERTY(IScreenSpaceReflectionEffect, sampling_bias)
        .RTTR_MEMBER_PROPERTY(IScreenSpaceReflectionEffect, thickness)
        .RTTR_MEMBER_PROPERTY(IScreenSpaceReflectionEffect, border_fade);
}

void IEffect::register_type() {
    rttr::registration::class_<IEffect>("ars::render::IEffect")
        .RTTR_MEMBER_PROPERTY_READONLY(IEffect, screen_space_reflection)
        .RTTR_MEMBER_PROPERTY_READONLY(IEffect, background);
}

void ISky::register_type() {
    rttr::registration::class_<ISky>("ars::render::ISky");
}

void IPanoramaSky::register_type() {
    rttr::registration::class_<IPanoramaSky>("ars::render::IPanoramaSky")
        .RTTR_MEMBER_PROPERTY(IPanoramaSky, panorama)
        .RTTR_MEMBER_PROPERTY(IPanoramaSky, irradiance_cube_map_size);
}

void IPhysicalSky::register_type() {
    rttr::registration::class_<IPhysicalSky>("ars::render::IPhysicalSky");
}

void IBackground::register_type() {
    rttr::registration::enumeration<BackgroundMode>(
        "ars::render::BackgroundMode")( //
        RTTR_ENUM_VALUE(BackgroundMode, Color),
        RTTR_ENUM_VALUE(BackgroundMode, Sky));

    rttr::registration::class_<IBackground>("ars::render::IBackground")
        .RTTR_MEMBER_PROPERTY(IBackground, mode)
        .RTTR_MEMBER_PROPERTY(IBackground, color)
        .RTTR_MEMBER_PROPERTY(IBackground, strength)
        .RTTR_MEMBER_PROPERTY(IBackground, sky);
}
} // namespace ars::render