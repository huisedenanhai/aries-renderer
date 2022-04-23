#if defined(ARS_METALLIC_ROUGHNESS_PBR_GEOMETRY_PASS)
#include "MetallicRoughnessPBR.glsl"
#elif defined(ARS_DEPTH_ONLY_PASS) || defined(ARS_OBJECT_ID_PASS)
#include "DepthID.glsl"
#endif
