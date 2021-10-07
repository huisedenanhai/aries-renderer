#version 450

layout(location = 0) in vec3 position_os;
layout(location = 1) in vec2 uv_os;

layout(push_constant) uniform PushConstant {
    mat4 MVP;
};

layout(location = 0) out vec2 uv_cs;

void main() {
    gl_Position = MVP * vec4(position_os, 1.0);
    uv_cs = uv_os;
}
