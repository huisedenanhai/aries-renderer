#version 450
#extension GL_ARB_separate_shader_objects : enable

#include "builtin/attributes.glsl"
#include "builtin/descriptors.glsl"

layout(ARS_VERTEX_POSITION) in vec3 position_os;
layout(ARS_VERTEX_NORMAL) in vec3 normal_os;
layout(location = 0) out vec3 color_vs;

void main() {
  gl_Position = vec4(vec4(position_os, 1.0) * MV, 1.0) * per_frame.P;
  color_vs = vec3(dot(normal_os, normalize(vec3(1.0))));
}