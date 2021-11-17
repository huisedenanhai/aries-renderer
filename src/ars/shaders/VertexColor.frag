#version 450

layout(location = 0) in vec4 color_vert;

layout(location = 0) out vec4 color;

void main() {
    color = color_vert;
}