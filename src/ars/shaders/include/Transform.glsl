#ifndef ARS_TRANSFORM_GLSL
#define ARS_TRANSFORM_GLSL

vec3 transform_normal(mat4 mat_inverse, vec3 n) {
    return vec3(dot(mat_inverse[0].xyz, n),
                dot(mat_inverse[1].xyz, n),
                dot(mat_inverse[2].xyz, n));
}

vec3 transform_vector(mat4 mat, vec3 v) {
    return mat[0].xyz * v.x + mat[1].xyz * v.y + mat[2].xyz * v.z;
}

vec4 transform_position(mat4 mat, vec3 p) {
    return mat * vec4(p, 1.0);
}

vec3 calculate_bitangent(vec3 normal, vec4 tangent) {
    return cross(normal, tangent.xyz) * tangent.w;
}

// Reconstruct position from screen space UV and sampled inverse Z depth buffer
// value
vec3 reconstruct_position_from_ss(mat4 inverse_p, vec2 uv, float depth01) {
    vec3 pos_hclip = vec3(uv * 2.0 - 1.0, depth01);
    vec4 pos = transform_position(inverse_p, pos_hclip);
    return pos.xyz / pos.w;
}

vec3 transform_position_hclip_to_ss(vec4 p) {
    p.xyz /= p.w;
    return vec3(p.xy * 0.5 + 0.5, p.z);
}

struct ShadingPoint {
    vec2 screen_uv;
    float depth01;
    vec3 pos_vs;
    // v_vs points from shading point to view point
    vec3 v_vs;
    vec3 v_ws;
};

ShadingPoint get_shading_point(vec2 uv, float depth01, mat4 I_P, mat4 I_V) {
    ShadingPoint info;
    info.screen_uv = uv;
    info.depth01 = depth01;
    info.pos_vs = reconstruct_position_from_ss(I_P, uv, info.depth01);
    info.v_vs = -normalize(info.pos_vs);
    info.v_ws = transform_vector(I_V, info.v_vs);
    return info;
}

vec3 calculate_geometry_normal_vs_from_depth_buffer(sampler2D depth_tex,
                                                    vec2 uv,
                                                    vec2 inv_resolution,
                                                    mat4 I_P,
                                                    float depth_level) {
    float depth01 = textureLod(depth_tex, uv, depth_level).r;

    vec3 pos_vs = reconstruct_position_from_ss(I_P, uv, depth01);

    vec2 uv_x = uv + vec2(inv_resolution.x, 0);
    vec3 pos_vs_x = reconstruct_position_from_ss(
        I_P, uv_x, textureLod(depth_tex, uv_x, depth_level).r);
    vec2 uv_y = uv + vec2(0, inv_resolution.y);
    vec3 pos_vs_y = reconstruct_position_from_ss(
        I_P, uv_y, textureLod(depth_tex, uv_y, depth_level).r);
    vec3 geom_normal_vs =
        normalize(cross(pos_vs_y - pos_vs, pos_vs_x - pos_vs));
    return geom_normal_vs;
}

vec3 calculate_geometry_normal_vs_from_depth_buffer(sampler2D depth_tex,
                                                    vec2 uv,
                                                    vec2 inv_resolution,
                                                    mat4 I_P) {
    return calculate_geometry_normal_vs_from_depth_buffer(
        depth_tex, uv, inv_resolution, I_P, 0);
}

#endif