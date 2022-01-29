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

float min_comp_value(vec4 v) {
    return min(min(v.x, v.y), min(v.z, v.w));
}

float max_comp_value(vec4 v) {
    return max(max(v.x, v.y), max(v.z, v.w));
}

bool point_inside_unit_rect(vec2 p) {
    bool x1 = p.x >= 0;
    bool y1 = p.y >= 0;
    bool x2 = p.x <= 1.0;
    bool y2 = p.y <= 1.0;

    return x1 && y1 && x2 && y2;
}

bool point_inside_unit_cube(vec3 p) {
    // Expand the sub condition results in faster code on M1.
    bool x1 = p.x >= 0;
    bool y1 = p.y >= 0;
    bool z1 = p.z >= 0;
    bool x2 = p.x <= 1.0;
    bool y2 = p.y <= 1.0;
    bool z2 = p.z <= 1.0;

    return x1 && y1 && z1 && x2 && y2 && z2;
}

#endif