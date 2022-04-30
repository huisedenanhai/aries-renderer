#ifndef ARS_MISC_GLSL
#define ARS_MISC_GLSL

// Constants

const float PI = 3.14159;
const float INV_PI = 1.0 / PI;

// Utility methods

float square(float v) {
    return v * v;
}

vec3 safe_normalize(vec3 v) {
    return dot(v, v) == 0 ? v : normalize(v);
}

float safe_sqrt(float v) {
    return sqrt(max(v, 0.0));
}

// Convert a integral index in texture space to equivalent UV
vec2 index_to_uv(vec2 index, vec2 inv_resolution) {
    // Notice the offset 0.5
    return (index + vec2(0.5)) * inv_resolution;
}

vec3 index_to_uv(vec3 index, vec3 inv_resolution) {
    return (index + vec3(0.5)) * inv_resolution;
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

vec2 panorama_direction_to_uv(vec3 dir) {
    vec2 angle = vec2(atan(dir.z, dir.x), asin(dir.y));
    return vec2(0.5, -1.0) / PI * angle + 0.5;
}

vec3 panorama_uv_to_direction(vec2 uv) {
    vec2 angle = PI * vec2(2.0, -1.0) * (uv - 0.5);
    float sin_phi = sin(angle.y);
    float cos_phi = cos(angle.y);
    float sin_theta = sin(angle.x);
    float cos_theta = cos(angle.x);
    return vec3(cos_theta * cos_phi, sin_phi, sin_theta * cos_phi);
}

void construct_TBN_with_normal(vec3 n, out vec3 t, out vec3 b) {
    vec3 v = abs(n.y) < 0.96 ? vec3(0.0, 1.0, 0.0) : vec3(1.0, 0.0, 0.0);
    t = normalize(cross(n, v));
    b = normalize(cross(n, t));
}

float min_comp_value(vec4 v) {
    return min(min(v.x, v.y), min(v.z, v.w));
}

float max_comp_value(vec4 v) {
    return max(max(v.x, v.y), max(v.z, v.w));
}

float min_comp_value(vec2 v) {
    return min(v.x, v.y);
}

float max_comp_value(vec2 v) {
    return max(v.x, v.y);
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

bool value_in_01(float v) {
    bool v1 = v >= 0.0;
    bool v2 = v <= 1.0;
    return v1 && v2;
}

float fade_on_edge_01(float v, float fade_size) {
    return clamp(min(v, 1.0 - v) / fade_size, 0.0, 1.0);
}

// check if there is any hit with sphere
// mu = dot(r_dir, ray_dir)
// this method ensures t0 <= t1
bool ray_hit_sphere(
    float radius, float r, float mu, out float t0, out float t1) {
    float d2 = square(radius) + square(r) * (square(mu) - 1.0);
    float d = safe_sqrt(d2);

    t0 = -d - r * mu;
    t1 = d - r * mu;
    return d2 >= 0;
}

// check if there is a hit in the positive direction of the ray
// mu = dot(r_dir, ray_dir)
bool ray_forward_hit_sphere(float radius, float r, float mu, out float dist) {
    float t0, t1;
    bool has_hit = ray_hit_sphere(radius, r, mu, t0, t1);

    dist = t0 >= 0 ? t0 : t1;

    return has_hit && dist >= 0;
}

float average(vec3 v) {
    return (v.x + v.y + v.z) / 3.0;
}

float inverse_lerp(float a, float b, float v) {
    return (v - a) / (b - a);
}

#endif