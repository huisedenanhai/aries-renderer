#ifndef ARS_MISC_GLSL
#define ARS_MISC_GLSL

// Constants

const float PI = 3.14159;

// Utility methods

float square(float v) {
    return v * v;
}

vec3 safe_normalize(vec3 v) {
    return dot(v, v) == 0 ? v : normalize(v);
}

// Convert a integral index in texture space to equivalent UV
vec2 index_to_uv(vec2 index, vec2 inv_resolution) {
    // Notice the offset 0.5
    return (index + vec2(0.5)) * inv_resolution;
}

float clamp01(float v) {
    return clamp(v, 0.0, 1.0);
}

#endif