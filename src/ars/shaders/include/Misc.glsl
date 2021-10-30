#ifndef ARS_MISC_GLSL
#define ARS_MISC_GLSL

vec3 safe_normalize(vec3 v) {
    return dot(v, v) == 0 ? v : normalize(v);
}

vec2 index_to_uv(vec2 index, vec2 inv_resolution) {
    // Notice the offset 0.5
    return (index + vec2(0.5)) * inv_resolution;
}

#endif