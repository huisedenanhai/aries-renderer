#version 450 core

layout(location = 0) out vec4 color;

layout(set = 0, binding = 0) uniform sampler2D tex;

layout(location = 0) in struct {
    vec4 color;
    vec2 uv;
} In;

void main() {
    color = In.color * texture(tex, In.uv.st);
}
