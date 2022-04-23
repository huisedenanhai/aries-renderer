// To use this file
//
// If material parameter is needed
// 1. Declare struct Material
// 2. define ARS_MATERIAL_SAMPLER_2D_COUNT if the material needs some texture
//
// If there needs no material parameter, define ARS_EMPTY_MATERIAL
//
// If a default vertex shader is all you need, define
// ARS_DEFINE_DEFAULT_VERTEX_SHADER

#extension GL_EXT_nonuniform_qualifier : require

#include <Misc.glsl>
#include <Transform.glsl>

// Shader input attrs
#ifdef FRILL_SHADER_STAGE_FRAG
layout(location = 0) flat in uint ars_in_instance_id;
#else
layout(location = 0) in uint ars_in_instance_id;
layout(location = 0) out uint ars_out_instance_id;
#endif

#ifdef FRILL_SHADER_STAGE_VERT

layout(location = 1) in vec3 in_position_os;
layout(location = 2) in vec3 in_normal_os;
layout(location = 3) in vec4 in_tangent_os;
layout(location = 4) in vec2 in_uv;

#ifdef ARS_SKINNED
layout(location = 5) in uvec4 in_joints;
layout(location = 6) in vec4 in_weights;
#endif

#endif

struct Instance {
    mat4 MV;
    mat4 I_MV;
    uint material_id;
    uint instance_id;
};

layout(set = 0, binding = 0) uniform View {
    ViewTransform ars_view;
};

ViewTransform get_view() {
    return ars_view;
}

layout(set = 0, binding = 1, std430) buffer Instances {
    Instance ars_instances[];
};

Instance get_instance() {
    return ars_instances[ars_in_instance_id];
}

#ifndef ARS_EMPTY_MATERIAL

layout(set = 0, binding = 2, std430) buffer Materials {
    Material ars_materials[];
};

Material get_material() {
    return ars_materials[get_instance().material_id];
}

#if defined(ARS_MATERIAL_SAMPLER_2D_COUNT) && ARS_MATERIAL_SAMPLER_2D_COUNT > 0

layout(set = 0, binding = 3) uniform sampler2D
    ars_samplers_2d[ARS_MATERIAL_SAMPLER_2D_COUNT];

vec4 sample_tex_2d(uint index, vec2 uv) {
    return texture(ars_samplers_2d[index], uv);
}

#endif

#endif

#ifdef ARS_SKINNED
layout(set = 1, binding = 0, std430) buffer Skin {
    mat4 ars_skin[];
};
#endif

#ifdef ARS_DEFINE_DEFAULT_VERTEX_SHADER
#ifdef FRILL_SHADER_STAGE_VERT

layout(location = 1) out vec3 out_position_vs;
layout(location = 2) out vec3 out_normal_vs;
layout(location = 3) out vec3 out_tangent_vs;
layout(location = 4) out vec3 out_bitangent_vs;
layout(location = 5) out vec2 out_uv;

void main() {
    Instance inst = get_instance();
    vec3 bitangent_os = calculate_bitangent(in_normal_os, in_tangent_os);

    mat4 MV = inst.MV;
    mat4 I_MV = inst.I_MV;

#ifdef ARS_SKINNED
    // Skinned mesh should ignore model transform, otherwise the parent xform
    // will be applied twice to vertices
    mat4 skin_mat = ars_skin[in_joints.x] * in_weights.x +
                    ars_skin[in_joints.y] * in_weights.y +
                    ars_skin[in_joints.z] * in_weights.z +
                    ars_skin[in_joints.w] * in_weights.w;
    MV = get_view().V * skin_mat;
    I_MV = inverse(skin_mat);
#endif

    vec4 pos_vs = transform_position(MV, in_position_os);
    out_position_vs = pos_vs.xyz / pos_vs.w;
    out_normal_vs = transform_normal(I_MV, in_normal_os);
    out_bitangent_vs = transform_vector(MV, bitangent_os);
    out_tangent_vs = transform_vector(MV, in_tangent_os.xyz);
    out_uv = in_uv;

    gl_Position = transform_position(get_view().P, out_position_vs);
}

#endif
#endif
