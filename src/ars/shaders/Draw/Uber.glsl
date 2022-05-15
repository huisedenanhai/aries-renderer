#version 450 core

#include <GBuffer.glsl>
#include <MetallicRoughnessPBR.glsl>
#include <ShadingModel.glsl>

#define ARS_BINDLESS_RESOURCE_SET 2
#include "MaterialAPI.glsl"

#include "Material.glsl"

#define ARS_DEFINE_DEFAULT_VERTEX_SHADER
#include "Draw.glsl"

// Frag shader
#ifdef FRILL_SHADER_STAGE_FRAG

layout(location = 1) in vec3 in_position_vs;
layout(location = 2) in vec3 in_normal_vs;
layout(location = 3) in vec3 in_tangent_vs;
layout(location = 4) in vec3 in_bitangent_vs;
layout(location = 5) in vec2 in_uv;

#ifdef ARS_GEOMETRY_PASS
layout(location = 0) out vec4 gbuffer0;
layout(location = 1) out vec4 gbuffer1;
layout(location = 2) out vec4 gbuffer2;
layout(location = 3) out vec4 gbuffer3;
#endif

#ifdef ARS_OBJECT_ID_PASS
layout(location = 0) out uint out_id;
#endif

void main() {
    SurfaceAttribute attr;
    attr.position = in_position_vs;
    attr.normal = in_normal_vs;
    attr.tangent = in_tangent_vs;
    attr.bitangent = in_bitangent_vs;
    attr.uv = in_uv;
    attr.front_facing = gl_FrontFacing;

    Material m = get_material();

    Closure c;
    if (!material_get_closure(m, attr, c)) {
        discard;
    }

#ifdef ARS_GEOMETRY_PASS
    GBuffer g;
    closure_get_gbuffer(c, g);
    encode_gbuffer(g, gbuffer0, gbuffer1, gbuffer2, gbuffer3);
#endif

#ifdef ARS_OBJECT_ID_PASS
    out_id = get_instance().custom_id;
#endif
}

#endif