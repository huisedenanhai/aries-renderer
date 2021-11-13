#version 450

layout(location = 0) in vec2 uv;

layout(location = 0) out uint color;

layout(set = 0, binding = 0) uniform sampler2D tex;
layout(set = 0, binding = 1) uniform Param {
    uint color_id;
    float alpha_clip;
};

void main() {
    if (texture(tex, uv).a <= alpha_clip) {
        discard;
    }
    color = color_id;
}