#version 450

layout(location = 0) in vec2 uv;

layout(location = 0) out vec4 color;

// layout(set = 0, binding = 0) uniform sampler2D tex;

void main() {
    // vec4 value = texture(tex, uv);
    vec4 value = vec4(uv, 0.0, 1.0);
    // Gamma correction to make result looks linear
    color = vec4(pow(value.xyz, vec3(2.2)), value.a);
}