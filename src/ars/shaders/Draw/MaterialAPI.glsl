#extension GL_EXT_nonuniform_qualifier : require

// #ifdef ARS_SUPPORT_BINDLESS
// #define ARS_BINDLESS_SAMPLER_2D_COUNT
// #else
// If the platform does not have proper bindless support, use simple descriptor
// indexing, and give bindless resources count an upper bound
#define ARS_BINDLESS_SAMPLER_2D_COUNT 8
// #endif

layout(set = 0, binding = 3) uniform sampler2D
    ars_samplers_2d[ARS_BINDLESS_SAMPLER_2D_COUNT];

vec4 sample_tex_2d(uint index, vec2 uv) {
    return texture(ars_samplers_2d[index], uv);
}

struct SurfaceAttribute {
    vec3 position;
    vec3 normal;
    vec3 tangent;
    vec3 bitangent;
    vec2 uv;
    bool front_facing;
};
