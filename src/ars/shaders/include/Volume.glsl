#ifndef ARS_VOLUME_GLSL
#define ARS_VOLUME_GLSL

#include <Misc.glsl>

// Convention:
// wi points from hit point to income direction, wo points from hit point to
// outcome direction, cos_theta = dot(wi, wo)

float rayleigh_phase(float cos_theta) {
    float a = 3.0 / (16.0 * PI);
    return a * (1.0 + square(cos_theta));
}

float mie_phase_cornette_shanks(float g, float cos_theta) {
    float a = 3.0 / (8.0 * PI);
    float g2 = square(g);
    float gf = (1.0 - g2) / (2.0 + g2);
    return a * gf * (1 + square(cos_theta)) /
           pow((1 + g2 + 2.0 * g * cos_theta), 1.5);
}

float isotropic_phase() {
    return 1.0 / (4.0 * PI);
}

#endif