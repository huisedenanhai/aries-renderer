#ifndef ARIES_BUILTIN_ATTRIBUTES
#define ARIES_BUILTIN_ATTRIBUTES

// per vertex
#define ARS_VERTEX_POSITION location = 0
#define ARS_VERTEX_NORMAL location = 1
#define ARS_VERTEX_TANGENT location = 2
#define ARS_VERTEX_UV0 location = 3
#define ARS_VERTEX_UV1 location = 4
#define ARS_VERTEX_COLOR location = 5

// per instance
layout(location = 8) in mat3x4 MV;
layout(location = 11) in mat3x4 I_MV;
layout(location = 14) in mat3x4 M;
layout(location = 17) in mat3x4 I_M;

#endif
