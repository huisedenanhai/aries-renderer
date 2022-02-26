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
    float ground_albedo;
    float mie_g;
};

// mu = dot(up, v_forward)
// Assume there is a hit
float distance_to_atmosphere_top(Atmosphere atm, float r, float mu) {
    float dist;
    bool hit = ray_hit_sphere(atm.top_radius, r, mu, dist);
    return max(0.0, dist);
}

float atmosphere_ray_sample_point_r(float r, float mu, float t) {
    return sqrt(r * r + t * t + 2.0 * r * t * mu);
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
    float rho = safe_sqrt(square(r) - square(atm.bottom_radius));
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

vec3 atmosphere_extinction_scatter(Atmosphere atm, float r) {
    float r_d = rayleigh_density(atm, r);
    float m_d = mie_density(atm, r);
    return atm.rayleigh_scattering * r_d + atm.mie_scattering * m_d;
}

vec3 atmosphere_extinction(Atmosphere atm, float r) {
    float r_d = rayleigh_density(atm, r);
    float m_d = mie_density(atm, r);
    float o_d = ozone_density(atm, r);
    return atm.rayleigh_scattering * r_d +
           (atm.mie_absorption + atm.mie_scattering) * m_d +
           atm.ozone_absorption * o_d;
}

vec2 multi_scattering_lut_uv_to_r_mu(Atmosphere atm, vec2 uv) {
    float r = uv.x * (atm.top_radius - atm.bottom_radius) + atm.bottom_radius;
    float mu = 2.0 * uv.y - 1.0;
    return vec2(r, mu);
}

vec2 multi_scattering_lut_r_mu_to_uv(Atmosphere atm, float r, float mu) {
    float x = (r - atm.bottom_radius) / (atm.top_radius - atm.bottom_radius);
    float y = 0.5 + 0.5 * mu;
    return vec2(x, y);
}

vec2 sky_view_lut_direction_to_uv(vec3 dir) {
    vec2 uv = panorama_direction_to_uv(dir);
    uv.y -= 0.5;
    uv.y = 0.5 + 0.5 * sign(uv.y) * sqrt(abs(uv.y) * 2.0);
    return uv;
}

vec3 sky_view_lut_uv_to_direction(vec2 uv) {
    uv.y -= 0.5;
    uv.y = 0.5 + 0.5 * sign(uv.y) * square(abs(uv.y) * 2.0);
    return panorama_uv_to_direction(uv);
}

#endif