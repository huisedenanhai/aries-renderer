#version 450 core

#include <GBuffer.glsl>
#include <Light.glsl>
#include <MetallicRoughnessPBR.glsl>
#include <Misc.glsl>
#include <Sampling.glsl>
#include <ShadingModel.glsl>
#include <Transform.glsl>

layout(location = 0) in vec2 uv;
layout(location = 0) out vec4 lit_result;

layout(set = 0, binding = 0) uniform sampler2D gbuffer0_tex;
layout(set = 0, binding = 1) uniform sampler2D gbuffer1_tex;
layout(set = 0, binding = 2) uniform sampler2D gbuffer2_tex;
layout(set = 0, binding = 3) uniform sampler2D gbuffer3_tex;
layout(set = 0, binding = 4) uniform sampler2D depth_stencil_tex;

layout(set = 0, binding = 5) uniform View {
    ViewTransform view;
};

#if defined(LIT_REFLECTION_EMISSION)

layout(set = 1, binding = 0) uniform Param {
    vec3 env_randiance_factor;
    int cube_map_mip_count;
};

layout(set = 1, binding = 1) uniform sampler2D brdf_lut;
layout(set = 1, binding = 2) uniform samplerCube cube_map;
layout(set = 1, binding = 3) uniform sampler2D ssr_reflection;

vec3 get_env_filtered_radiance(vec3 v_ws, float lod) {
    return env_randiance_factor * textureLod(cube_map, v_ws, lod).rgb;
}

vec3 get_diffuse_irradiance(vec3 n_ws) {
    return get_env_filtered_radiance(n_ws, cube_map_mip_count - 1) * PI;
}

vec3 get_glossy_irradiance(ShadingPoint sp,
                           vec3 n_ws,
                           float perceptual_roughness) {
    vec3 reflect_dir_ws = reflect(-sp.v_ws, n_ws);
    vec3 filtered_radiance = get_env_filtered_radiance(
        reflect_dir_ws, (cube_map_mip_count - 1) * perceptual_roughness);

    vec4 ssr = texture(ssr_reflection, sp.screen_uv);
    filtered_radiance = mix(filtered_radiance, ssr.rgb, ssr.a);

    return filtered_radiance * PI;
}

#endif

// Light position and direction are in view space
#if defined(LIT_POINT_LIGHT)
layout(set = 1, binding = 0) uniform Param {
    PointLight light;
};
#endif

#if defined(LIT_DIRECTIONAL_LIGHT) || defined(LIT_SUN)
layout(set = 1, binding = 0) uniform Param {
    DirectionalLight light;
};

const int SHADOW_CASCADE_COUNT = 4;

struct ShadowCascade {
    mat4 view_to_shadow_hclip;
    vec4 uv_clamp;
    float z_near;
    float z_far;
};

layout(set = 2, binding = 0) uniform ShadowParam {
    ShadowCascade shadow_cascades[SHADOW_CASCADE_COUNT];
};

layout(set = 2, binding = 1) uniform sampler2D shadow_map_tex;

vec3 sample_shadow_pcf(vec2 uv, float ref_z) {
    // clamp ref_z to positive to avoid incorrect shadow when
    // point is out of light far plane.
    ref_z = max(ref_z, 0.0);
    vec2 index = uv * textureSize(shadow_map_tex, 0);
    vec2 center = floor(index);

    float visibility_acc = 0.0;
    float weight_acc = 0.0;

    for (int x = -1; x <= 1; x++) {
        for (int y = -1; y <= 1; y++) {
            float shadow01 =
                texelFetch(shadow_map_tex, ivec2(center) + ivec2(x, y), 0).r;
            // 3x3 tent filter
            float weight = max(
                1.5 - max_comp_value(abs(vec2(x, y) + center + 0.5 - index)),
                0.0);
            visibility_acc += (shadow01 <= ref_z ? weight : 0.0);
            weight_acc += weight;
        }
    }
    return vec3(visibility_acc / weight_acc);
}

vec3 get_shadow_factor(ShadingPoint sp) {
    int i = 0;
    for (; i < SHADOW_CASCADE_COUNT; i++) {
        ShadowCascade cascade = shadow_cascades[i];
        if (-sp.pos_vs.z > cascade.z_far && i + 1 != SHADOW_CASCADE_COUNT) {
            // Don't discard shadow at the last cascade, to avoid incorrect
            // shadow cutoff when visible far sample distance increase abruptly.
            continue;
        }
        vec4 pos_shadow_hclip =
            transform_position(cascade.view_to_shadow_hclip, sp.pos_vs);
        vec3 pos_shadow_ss = transform_position_hclip_to_ss(pos_shadow_hclip);
        return sample_shadow_pcf(
            clamp(pos_shadow_ss.xy, cascade.uv_clamp.xy, cascade.uv_clamp.zw),
            pos_shadow_ss.z);
    }
    return vec3(1.0);
}

#endif

#if defined(LIT_SUN)

#include "../Atmosphere/AtmosphereCommon.glsl"

layout(set = 1, binding = 1) uniform AtmosphereParam {
    Atmosphere atmosphere;
};

layout(set = 1, binding = 2) uniform sampler2D transmittance_lut;

#endif

vec3 get_transmittance(ShadingPoint sp, vec3 l_vs) {
    vec3 t = vec3(1.0);

#if defined(LIT_SUN)
    vec3 l_ws = transform_vector(view.I_V, l_vs);
    vec3 pos_planet = get_position_planet_coord(atmosphere, sp.pos_ws);
    vec3 r_dir = normalize(pos_planet);
    t *= get_transmittance(
        atmosphere, transmittance_lut, length(pos_planet), dot(r_dir, l_ws));
#endif

#if defined(LIT_DIRECTIONAL_LIGHT) || defined(LIT_SUN)
    t *= get_shadow_factor(sp);
#endif

    return t;
}

vec3 shade_metallic_roughness_pbr(GBuffer gbuffer, ShadingPoint sp) {
    MetallicRoughnessPBR brdf = get_metallic_roughness_pbr(gbuffer);

    vec3 fd = diffuse_BRDF(brdf);

    // Extract geometry info
    vec3 pos_vs = sp.pos_vs;
    vec3 v_vs = sp.v_vs;
    vec3 v_ws = sp.v_ws;
    vec3 n_vs = normalize(gbuffer.normal_vs);

    vec3 radiance = vec3(0.0);

#if defined(LIT_REFLECTION_EMISSION)
    // Sample env irradiance
    vec3 n_ws = transform_vector(view.I_V, n_vs);
    vec3 diffuse_env_irradiance = brdf.occlusion * get_diffuse_irradiance(n_ws);
    vec3 diffuse_indirect_radiance = fd * diffuse_env_irradiance;

    float NoV = clamp01(dot(n_vs, v_vs));
    vec3 fe = environment_BRDF(brdf, brdf_lut, NoV);
    vec3 glossy_env_irradiance =
        brdf.occlusion *
        get_glossy_irradiance(sp, n_ws, brdf.perceptual_roughness);
    vec3 glossy_indirect_radiance = fe * glossy_env_irradiance;

    radiance += diffuse_indirect_radiance + glossy_indirect_radiance;
    radiance += gbuffer.emission;
#endif

#if defined(LIT_DIRECTIONAL_LIGHT) || defined(LIT_POINT_LIGHT) ||              \
    defined(LIT_SUN)
    vec3 lr, l_vs;
    radiance_to_point(light, pos_vs, lr, l_vs);

    lr *= get_transmittance(sp, l_vs);

    float NoL = clamp01(dot(n_vs, l_vs));
    vec3 fr = specular_BRDF(brdf, n_vs, v_vs, l_vs);
    radiance += (fd + fr) * NoL * lr;
#endif

    return radiance;
}

vec3 shade(GBuffer gbuffer, ShadingPoint sp) {
#if defined(UNLIT)
    if (gbuffer.shading_model == SHADING_MODEL_UNLIT) {
        return gbuffer.base_color.rgb;
    }
#else
    if (gbuffer.shading_model == SHADING_MODEL_METALLIC_ROUGHNESS_PBR) {
        return shade_metallic_roughness_pbr(gbuffer, sp);
    }
#endif

    discard;
}

void main() {
    float depth01 = texture(depth_stencil_tex, uv).r;
    // Ignore background
    if (depth01 == 0.0) {
        discard;
    }

    GBuffer gbuffer = decode_gbuffer(texture(gbuffer0_tex, uv),
                                     texture(gbuffer1_tex, uv),
                                     texture(gbuffer2_tex, uv),
                                     texture(gbuffer3_tex, uv));

    ShadingPoint sp = get_shading_point(view, uv, depth01);
    lit_result = vec4(shade(gbuffer, sp), 1.0);
}