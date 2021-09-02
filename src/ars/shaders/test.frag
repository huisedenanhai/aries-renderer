#version 450
#extension GL_ARB_separate_shader_objects : enable

#include "builtin/descriptors.glsl"

layout(location = 0) in vec3 color_vs;

layout(location = 0) out vec4 fragment_color;

void main() {
  fragment_color = vec4(color_vs, 1.0);
}
