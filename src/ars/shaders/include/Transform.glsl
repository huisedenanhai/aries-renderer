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

#endif