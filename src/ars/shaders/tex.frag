#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 color_vs;
layout(location = 1) in vec2 uv_vs;

layout(set = 1, binding = 0) uniform sampler tex_sampler;
layout(set = 1, binding = 1) uniform texture2D tex;

layout(location = 0) out vec4 fragment_color;

void main() {
  fragment_color = vec4(texture(sampler2D(tex, tex_sampler), uv_vs).rgb, 1.0);
  // vec4(uv_vs, 0.0, 1.0);
}
