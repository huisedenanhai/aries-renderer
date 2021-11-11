#version 450

layout(location = 0) out uint color;

#include "ObjectIdInput.glsl"

void main() {
    color = color_id;
}
