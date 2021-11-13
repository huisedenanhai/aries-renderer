#version 450

layout(location = 0) out vec2 uv;

layout(push_constant) uniform PushConstant {
    mat4 MVP;
};

void main() {
    vec2 vertices[4] = {vec2(0.0, 0.0), //
                        vec2(1.0, 0.0),
                        vec2(0.0, 1.0),
                        vec2(1.0, 1.0)};
    int indices[6] = {0, 1, 3, 0, 3, 2};

    vec2 vert = vertices[indices[gl_VertexIndex]];
    uv = vert;

    gl_Position = MVP * vec4(vert - 0.5, 0.0, 1.0);
}