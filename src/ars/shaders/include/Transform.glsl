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

#endif