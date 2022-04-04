#version 450 core

#include <GBuffer.glsl>
#include <MetallicRoughnessPBR.glsl>
#include <ShadingModel.glsl>

struct Material {
    vec4 base_color_factor;
    float metallic_factor;
    float roughness_factor;
    float normal_scale;
    float occlusion_strength;
    vec3 emission_factor;
    uint base_color_tex;
    uint metallic_roughness_tex;
    uint normal_tex;
    uint occlusion_tex;
    uint emission_tex;
};

#define ARS_MATERIAL_SAMPLER_2D_COUNT 5

#define ARS_DEFINE_DEFAULT_VERTEX_SHADER
#include "Draw.glsl"

// Frag shader
#ifdef FRILL_SHADER_STAGE_FRAG

layout(location = 1) in vec3 in_position_vs;
layout(location = 2) in vec3 in_normal_vs;
layout(location = 3) in vec3 in_tangent_vs;
layout(location = 4) in vec3 in_bitangent_vs;
layout(location = 5) in vec2 in_uv;

layout(location = 0) out vec4 gbuffer0;
layout(location = 1) out vec4 gbuffer1;
layout(location = 2) out vec4 gbuffer2;
layout(location = 3) out vec4 gbuffer3;

vec3 get_shading_normal_ts(Material m) {
    vec3 normal = sample_tex_2d(m.normal_tex, in_uv).xyz * 2.0 - 1.0;
    return normal * vec3(m.normal_scale, m.normal_scale, 1.0);
}

// The result may be not normalized
vec3 get_shading_normal_vs(Material m) {
    vec3 normal_ts = get_shading_normal_ts(m);
    // avoid NAN when tangent is not present
    vec3 shading_normal_vs = normal_ts.x * safe_normalize(in_tangent_vs) +
                             normal_ts.y * safe_normalize(in_bitangent_vs) +
                             normal_ts.z * safe_normalize(in_normal_vs);
    return gl_FrontFacing ? shading_normal_vs : -shading_normal_vs;
}

float get_occlusion(Material m) {
    float occlusion = sample_tex_2d(m.occlusion_tex, in_uv).r;
    return mix(1.0, occlusion, m.occlusion_strength);
}

vec3 get_emission(Material m) {
    return sample_tex_2d(m.emission_tex, in_uv).rgb * m.emission_factor;
}

void main() {
    Material m = get_material();
    GBuffer g;

    g.base_color = sample_tex_2d(m.base_color_tex, in_uv) * m.base_color_factor;
    g.normal_vs = normalize(get_shading_normal_vs(m));

    MetallicRoughnessPBRGBuffer pbr;
    pbr.occlusion = get_occlusion(m);

    vec4 metallic_roughness = sample_tex_2d(m.metallic_roughness_tex, in_uv);
    pbr.metallic = metallic_roughness.b * m.metallic_factor;
    pbr.perceptual_roughness = metallic_roughness.g * m.roughness_factor;

    g.material = encode_material(pbr);
    g.shading_model = SHADING_MODEL_METALLIC_ROUGHNESS_PBR;

    g.emission = get_emission(m);

    encode_gbuffer(g, gbuffer0, gbuffer1, gbuffer2, gbuffer3);
}

#endif