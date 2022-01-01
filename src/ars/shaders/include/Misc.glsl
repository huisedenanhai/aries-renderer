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

vec3 cube_map_index_to_direction(ivec3 index, int size) {
    float half_size = size * 0.5;
    vec2 local_coord = index.xy - half_size + 0.5;
    vec3 dirs[6] = {
        vec3(half_size, -local_coord.y, -local_coord.x),
        vec3(-half_size, -local_coord.y, local_coord.x),
        vec3(local_coord.x, half_size, local_coord.y),
        vec3(local_coord.x, -half_size, -local_coord.y),
        vec3(local_coord.x, -local_coord.y, half_size),
        vec3(-local_coord.x, -local_coord.y, -half_size),
    };

    return normalize(dirs[index.z]);
}

vec2 hdr_direction_to_uv(vec3 dir) {
    vec2 angle = vec2(atan(dir.z, dir.x), -asin(dir.y));
    return vec2(0.5, 1.0) / PI * angle + 0.5;
}

void construct_TBN_with_normal(vec3 n, out vec3 t, out vec3 b) {
    vec3 v = n.y < 0.96 ? vec3(0.0, 1.0, 0.0) : vec3(1.0, 0.0, 0.0);
    t = normalize(cross(n, v));
    b = normalize(cross(n, t));
}

#endif