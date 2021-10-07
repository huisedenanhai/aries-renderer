#version 450

layout(location = 0) in vec2 uv_cs;

layout(set = 0, binding = 0) uniform sampler2D base_color_tex;

layout(location = 0) out vec4 color;

void main() {
    color = texture(base_color_tex, uv_cs);
}
