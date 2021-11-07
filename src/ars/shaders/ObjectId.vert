#version 450

layout(location = 0) in vec3 position_os;

#include "ObjectIdInput.glsl"

void main() {
    gl_Position = MVP * vec4(position_os, 1.0);
}
