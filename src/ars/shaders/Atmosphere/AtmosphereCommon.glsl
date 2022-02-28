#ifndef ARS_ATMOSPHERE_COMMON_GLSL
#define ARS_ATMOSPHERE_COMMON_GLSL

#include <Misc.glsl>
#include <Transform.glsl>
#include <Volume.glsl>

// By default Atmosphere calculation use km as length unit.

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

struct AtmosphereSun {
    // points from shading point to sun
    vec3 direction;
    vec3 radiance;
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

// view_dir points from view point to shading point.
// sun_dir points from shading point to sun.
// Directions and positions are in the planet world coordinate, which origin is
// at the center of planet.
void ray_march_scattering_transmittance(Atmosphere atm,
                                        AtmosphereSun sun,
                                        sampler2D transmittance_lut,
                                        sampler2D multi_scattering_lut,
                                        vec3 view_dir,
                                        vec3 view_pos,
                                        float dist,
                                        int step_count,
                                        out vec3 scattering,
                                        out vec3 transmittance) {
    transmittance = vec3(1.0);
    scattering = vec3(0.0);

    float dt = dist / step_count;
    for (int i = 0; i < step_count; i++) {
        float t = (i + 0.5) * dt;
        vec3 ray_p = view_pos + view_dir * t;
        float r = length(ray_p);
        float mu = dot(normalize(ray_p), sun.direction);

        vec3 s =
            texture(transmittance_lut, transmittance_lut_r_mu_to_uv(atm, r, mu))
                .rgb;
        vec3 ms = texture(multi_scattering_lut,
                          multi_scattering_lut_r_mu_to_uv(atm, r, mu))
                      .rgb;

        vec3 e_s_r = rayleigh_density(atm, r) * atm.rayleigh_scattering;
        float e_s_m = mie_density(atm, r) * atm.mie_scattering;

        float cos_scatter = dot(sun.direction, -view_dir);
        float phase_r = rayleigh_phase(cos_scatter);
        float phase_m = mie_phase_cornette_shanks(atm.mie_g, cos_scatter);

        vec3 e = atmosphere_extinction(atm, r);

        vec3 direct_scatter = (e_s_r * phase_r + e_s_m * phase_m) * s;
        vec3 multi_scatter = (e_s_r + e_s_m) * ms;
        scattering += transmittance * (direct_scatter + multi_scatter) * dt;
        transmittance *= exp(-e * dt);
    }

    scattering = max(scattering * sun.radiance, vec3(0.0));
}

vec3 get_view_position_planet_coord(Atmosphere atm, ViewTransform view) {
    vec3 eye_pos_ws = get_eye_position_ws(view);

    return vec3(0.0,
                clamp(atm.bottom_radius + eye_pos_ws.y * 1e-3,
                      atm.bottom_radius,
                      atm.top_radius),
                0.0);
}

#endif