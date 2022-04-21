#version 450 core

#include <GBuffer.glsl>
#include <MetallicRoughnessPBR.glsl>
#include <ShadingModel.glsl>

#define ARS_UBER_SHADER_DEFINE_MATERIAL
#include "UberPass.glsl"
#undef ARS_UBER_SHADER_DEFINE_MATERIAL

#define ARS_DEFINE_DEFAULT_VERTEX_SHADER
#include "Draw.glsl"

// Frag shader
#ifdef FRILL_SHADER_STAGE_FRAG

layout(location = 1) in vec3 in_position_vs;
layout(location = 2) in vec3 in_normal_vs;
layout(location = 3) in vec3 in_tangent_vs;
layout(location = 4) in vec3 in_bitangent_vs;
layout(location = 5) in vec2 in_uv;

#define ARS_UBER_SHADER_DEFINE_FRAG_SHADER
#include "UberPass.glsl"
#undef ARS_UBER_SHADER_DEFINE_FRAG_SHADER

#endif