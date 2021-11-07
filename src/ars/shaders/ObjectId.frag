#version 450

layout(location = 0) out vec4 color;

#include "ObjectIdInput.glsl"

void main() {
    color = color_id;
}
