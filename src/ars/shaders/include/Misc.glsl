#ifndef ARS_MISC_GLSL
#define ARS_MISC_GLSL

vec3 safe_normalize(vec3 v) {
    return dot(v, v) == 0 ? v : normalize(v);
}

#endif