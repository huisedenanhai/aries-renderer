#version 450

layout(location = 0) out vec2 uv;

void main() {
    vec2 vertices[3] = {vec2(-1.0, -1.0), vec2(3.0, -1.0), vec2(-1.0, 3.0)};
    vec2 uvs[3] = {vec2(0.0, 0.0), vec2(2.0, 0.0), vec2(0.0, 2.0)};

    vec2 vert = vertices[gl_VertexIndex];
    uv = uvs[gl_VertexIndex];

    gl_Position = vec4(vert, 0.0, 1.0);
}