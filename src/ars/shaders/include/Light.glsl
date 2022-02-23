#ifndef ARS_LIGHT_GLSL
#define ARS_LIGHT_GLSL

#include <Misc.glsl>

struct PointLight {
    vec3 position;
    vec3 color;
    float intensity;
};

// Direction is from shading point to light
void radiance_to_point(PointLight light,
                       vec3 point,
                       out vec3 radiance,
                       out vec3 direction) {
    vec3 point_to_light = light.position - point;
    float attenuation = 1.0 / max(dot(point_to_light, point_to_light), 1e-5);
    radiance = attenuation * light.color * light.intensity / (4.0 * PI);
    direction = normalize(point_to_light);
}

struct DirectionalLight {
    // from shading point to light
    // direction should be normalized
    vec3 direction;
    vec3 color;
    float intensity;
};

void radiance_to_point(DirectionalLight light,
                       vec3 point,
                       out vec3 radiance,
                       out vec3 direction) {
    radiance = light.color * light.intensity;
    direction = light.direction;
}

#endif
