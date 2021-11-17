#version 450

layout(location = 0) in vec3 position_os;
layout(location = 1) in vec4 color_os;

layout(push_constant) uniform PushConstant {
    mat4 MVP;
};

layout(location = 0) out vec4 color_vert;

void main() {
    color_vert = color_os;
    gl_Position = MVP * vec4(position_os, 1.0);
}
