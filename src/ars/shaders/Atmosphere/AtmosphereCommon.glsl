#ifndef ARS_ATMOSPHERE_COMMON_GLSL
#define ARS_ATMOSPHERE_COMMON_GLSL

#include <Misc.glsl>

// By default Atmosphere calculation use km as length unit.
// Other unit should works fine as long as all unit is consistent.

struct Atmosphere {
    float bottom_radius;
    float top_radius;
    float mie_scattering;
    float mie_absorption;
    vec3 rayleigh_scattering;
    float rayleigh_altitude;
    vec3 ozone_absorption;
    float mie_altitude;
    float ozone_altitude;
    float ozone_thickness;
};

// mu = dot(up, v_forward)
float distance_to_atmosphere_top(Atmosphere atm, float r, float mu) {
    float d = sqrt(square(atm.top_radius) + square(r) * (square(mu) - 1.0));
    return max(0.0, d - r * mu);
}

vec2 transmittance_lut_uv_to_r_mu(Atmosphere atm, vec2 uv) {
    float h = sqrt(square(atm.top_radius) - square(atm.bottom_radius));
    float rho = uv.x * h;
    float r = sqrt(square(atm.bottom_radius) + square(rho));

    float d_min = atm.top_radius - r;
    float d_max = rho + h;
    float d = mix(d_min, d_max, uv.y);
    float mu = (square(atm.top_radius) - square(r) - square(d)) / (2.0 * r * d);

    return vec2(r, mu);
}

vec2 transmittance_lut_r_mu_to_uv(Atmosphere atm, float r, float mu) {
    float h = sqrt(square(atm.top_radius) - square(atm.bottom_radius));
    float rho = sqrt(square(r) - square(atm.bottom_radius));
    float x = rho / h;

    float d = distance_to_atmosphere_top(atm, r, mu);
    float d_min = atm.top_radius - r;
    float d_max = rho + h;
    float y = (d - d_min) / (d_max - d_min);

    return vec2(x, y);
}

float rayleigh_density(Atmosphere atm, float r) {
    float h = r - atm.bottom_radius;
    return exp(-h / atm.rayleigh_altitude);
}

float mie_density(Atmosphere atm, float r) {
    float h = r - atm.bottom_radius;
    return exp(-h / atm.mie_altitude);
}

float ozone_density(Atmosphere atm, float r) {
    float h = r - atm.bottom_radius;
    return max(0.0,
               1.0 - abs(h - atm.ozone_altitude) / (0.5 * atm.ozone_thickness));
}

vec3 atmosphere_extinction(Atmosphere atm, float r) {
    float r_d = rayleigh_density(atm, r);
    float m_d = mie_density(atm, r);
    float o_d = ozone_density(atm, r);
    return atm.rayleigh_scattering * r_d +
           (atm.mie_absorption + atm.mie_scattering) * m_d +
           atm.ozone_absorption * o_d;
}

#endif