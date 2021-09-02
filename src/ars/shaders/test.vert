#version 450
#extension GL_ARB_separate_shader_objects : enable

#include "builtin/attributes.glsl"
#include "builtin/descriptors.glsl"

layout(ARS_VERTEX_POSITION) in vec3 position_os;
layout(location = 0) out vec3 color_vs;

void main() {
  gl_Position =
      vec4(vec4(position_os.x, -position_os.y, position_os.z, 1.0) * MV, 1.0);
  gl_Position.z = 0;
  color_vs = per_frame.color.rgb;
}