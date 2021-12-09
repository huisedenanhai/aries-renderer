#version 450

#include <Transform.glsl>

layout(location = 0) in vec3 position_os;
layout(location = 1) in vec3 normal_os;
layout(location = 2) in vec4 tangent_os;
layout(location = 3) in vec2 uv_os;

layout(location = 0) out vec3 position_vs;
layout(location = 1) out vec3 normal_vs;
layout(location = 2) out vec3 bitangent_vs;
layout(location = 3) out vec3 tangent_vs;
layout(location = 4) out vec2 uv_vs;

layout(std140, set = 0, binding = 0) uniform Transform {
    mat4 MV;
    mat4 I_MV;
    mat4 P;
};

void main() {
    position_vs = transform_position(MV, position_os).xyz;
    normal_vs = transform_normal(I_MV, normal_os);
    vec3 bitangent_os = calculate_bitangent(normal_os, tangent_os);
    bitangent_vs = transform_vector(MV, bitangent_os);
    tangent_vs = transform_vector(MV, tangent_os.xyz);
    uv_vs = uv_os;

    gl_Position = transform_position(P, position_vs);
}
