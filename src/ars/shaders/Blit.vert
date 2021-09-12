#version 450

vec2 vertices[3] = {vec2(0.0, 0.0), vec2(2.0, 0.0), vec2(2.0, 0.0)};

layout(location = 0) out vec2 uv;

void main() {
    uv = vertices[gl_VertexIndex];
    gl_Position = vec4(uv, 0.0, 1.0);
}