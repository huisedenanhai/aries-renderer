#version 450
#extension GL_ARB_separate_shader_objects : enable

#include "builtin/attributes.glsl"
#include "builtin/descriptors.glsl"

layout(ARS_VERTEX_POSITION) in vec3 position_os;
layout(location = 0) out vec3 color_vs;
layout(location = 1) out vec2 uv_vs;

void main() {
  gl_Position = vec4(vec4(position_os + vec3(0.0, 0.0, -0.9), 1.0) * MV, 1.0) *
                per_frame.P;
  color_vs = per_frame.color.rgb;
  uv_vs = (position_os.xy * vec2(1, -1) + vec2(1.0)) * 0.5;
}