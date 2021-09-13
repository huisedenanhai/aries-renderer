#version 450

layout(location = 0) in vec2 uv;

layout(location = 0) out vec4 color;

void main() {
    vec3 linear_color = vec3(uv, 0.0);
    // Gamma correction to make result looks linear
    color = vec4(pow(linear_color, vec3(2.2)), 1.0);
}