#version 450

#include <GBuffer.glsl>
#include <MetallicRoughnessPBR.glsl>
#include <Misc.glsl>
#include <ShadingModel.glsl>

layout(location = 0) in vec3 positoin_vs;
layout(location = 1) in vec3 normal_vs;
layout(location = 2) in vec3 bitangent_vs;
layout(location = 3) in vec3 tangent_vs;
layout(location = 4) in vec2 uv_vs;

layout(location = 0) out vec4 gbuffer0;
layout(location = 1) out vec4 gbuffer1;
layout(location = 2) out vec4 gbuffer2;
layout(location = 3) out vec4 gbuffer3;

layout(std140, set = 1, binding = 0) uniform Material {
    vec4 base_color_factor;
    float metallic_factor;
    float roughness_factor;
    float normal_scale;
    float occlusion_strength;
    vec3 emission_factor;
};

layout(set = 1, binding = 1) uniform sampler2D base_color_tex;
layout(set = 1, binding = 2) uniform sampler2D metallic_roughness_tex;
layout(set = 1, binding = 3) uniform sampler2D normal_tex;
layout(set = 1, binding = 4) uniform sampler2D occlusion_tex;
layout(set = 1, binding = 5) uniform sampler2D emission_tex;

vec3 get_shading_normal_ts() {
    vec3 normal = texture(normal_tex, uv_vs).xyz * 2.0 - 1.0;
    return normal * vec3(normal_scale, normal_scale, 1.0);
}

vec3 get_shading_normal_vs() {
    vec3 normal_ts = get_shading_normal_ts();
    // avoid NAN when tangent is not present
    return normal_ts.x * safe_normalize(tangent_vs) +
           normal_ts.y * safe_normalize(bitangent_vs) +
           normal_ts.z * safe_normalize(normal_vs);
}

float get_occlusion() {
    float occlusion = texture(occlusion_tex, uv_vs).r;
    return mix(1.0, occlusion, occlusion_strength);
}

vec3 get_emission() {
    return texture(emission_tex, uv_vs).rgb * emission_factor;
}

void main() {
    GBuffer g;

    g.base_color = texture(base_color_tex, uv_vs) * base_color_factor;
    g.normal = normalize(get_shading_normal_vs());

    MetallicRoughnessPBRParam material;
    material.occlusion = get_occlusion();

    vec4 metallic_roughness = texture(metallic_roughness_tex, uv_vs);
    material.metallic = metallic_roughness.b * metallic_factor;
    material.perceptual_roughness = metallic_roughness.g * roughness_factor;

    g.material = encode_material(material);
    g.shading_model = SHADING_MODEL_METALLIC_ROUGHNESS_PBR;

    g.emission = get_emission();

    encode_gbuffer(g, gbuffer0, gbuffer1, gbuffer2, gbuffer3);
}
