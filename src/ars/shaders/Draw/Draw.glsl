#version 450 core

#extension GL_EXT_nonuniform_qualifier : require

#include <GBuffer.glsl>
#include <MetallicRoughnessPBR.glsl>
#include <Misc.glsl>
#include <ShadingModel.glsl>
#include <Transform.glsl>

#ifdef FRILL_SHADER_STAGE_FRAG
layout(location = 0) flat in uint ars_in_instance_id;
#else
layout(location = 0) in uint ars_in_instance_id;
layout(location = 0) out uint ars_out_instance_id;
#endif

struct Instance {
    mat4 MV;
    mat4 I_MV;
    uint material_id;
    uint instance_id;
};

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

layout(binding = 0) uniform View {
    ViewTransform ars_view;
};

layout(binding = 1, std430) buffer Instances {
    Instance ars_instances[];
};

layout(binding = 2, std430) buffer Materials {
    Material ars_materials[];
};

layout(binding = 3) uniform sampler2D ars_samplers_2d[];

Instance get_instance() {
    return ars_instances[ars_in_instance_id];
}

Material get_material() {
    return ars_materials[get_instance().material_id];
}

ViewTransform get_view() {
    return ars_view;
}

vec4 sample_tex_2d(uint index, vec2 uv) {
    return texture(ars_samplers_2d[index], uv);
}

// Vertex shader
#ifdef FRILL_SHADER_STAGE_VERT

layout(location = 1) in vec3 in_position_os;
layout(location = 2) in vec3 in_normal_os;
layout(location = 3) in vec4 in_tangent_os;
layout(location = 4) in vec2 in_uv;

layout(location = 1) out vec3 out_position_vs;
layout(location = 2) out vec3 out_normal_vs;
layout(location = 3) out vec3 out_tangent_vs;
layout(location = 4) out vec3 out_bitangent_vs;
layout(location = 5) out vec2 out_uv;

void main() {
    Instance inst = get_instance();
    vec3 bitangent_os = calculate_bitangent(in_normal_os, in_tangent_os);

    out_position_vs = transform_position(inst.MV, in_position_os).xyz;
    out_normal_vs = transform_normal(inst.I_MV, in_normal_os);
    out_bitangent_vs = transform_vector(inst.MV, bitangent_os);
    out_tangent_vs = transform_vector(inst.MV, in_tangent_os.xyz);
    out_uv = in_uv;

    gl_Position = transform_position(get_view().P, out_position_vs);
}

#endif

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