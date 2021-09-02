#ifndef ARIES_BUILTIN_DESCRIPTORS
#define ARIES_BUILTIN_DESCRIPTORS

layout(set = 0, binding = 0) uniform PerFrame {
  vec4 color;
  mat4 P;
}
per_frame;

#endif