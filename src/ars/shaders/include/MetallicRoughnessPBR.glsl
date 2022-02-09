#ifndef ARS_METALLIC_ROUGHNESS_PBR
#define ARS_METALLIC_ROUGHNESS_PBR

#include <GBuffer.glsl>
#include <Misc.glsl>

// GBuffer encoding for metallic roughness pbr
struct MetallicRoughnessPBRGBuffer {
    float occlusion;
    float metallic;
    float perceptual_roughness;
};

vec3 encode_material(MetallicRoughnessPBRGBuffer p) {
    return vec3(p.occlusion, p.metallic, p.perceptual_roughness);
}

MetallicRoughnessPBRGBuffer decode_material(vec3 m) {
    MetallicRoughnessPBRGBuffer p;
    p.occlusion = m.r;
    p.metallic = m.g;
    p.perceptual_roughness = m.b;

    return p;
}

// We follow the paper
// 'Microfacet Models for Refraction through Rough Surfaces'
// which introduce the GGX distribution to model the scattering of rough glass.
//
// We use SI(International System of Units) for physical terms.
float D_GGX(float NoH, float roughness) {
    float a2 = square(roughness);
    float c2 = square(NoH);
    return a2 / (PI * square(a2 * c2 + 1.0 - c2));
}

// V1 = G1 / (2 (n.v))
float V1_SmithGGX(float NoV, float roughness) {
    float a2 = square(roughness);
    return 1.0 / (NoV + sqrt(square(NoV) + a2 * (1.0 - square(NoV))));
}

// V = G / (4 (n.v) (n.l))
float V_SmithGGX(float NoV, float NoL, float roughness) {
    return V1_SmithGGX(NoV, roughness) * V1_SmithGGX(NoL, roughness);
}

vec3 F_Schlick(float LoH, vec3 f0) {
    float f = pow(1.0 - LoH, 5.0);
    return f + f0 * (1.0 - f);
}

struct MetallicRoughnessPBR {
    vec3 base_color;
    float metallic;
    float perceptual_roughness;
    float occlusion;
};

// Call this method only when
// gbuffer.shading_model == SHADING_MODEL_METALLIC_ROUGHNESS_PBR
MetallicRoughnessPBR get_metallic_roughness_pbr(GBuffer gbuffer) {
    MetallicRoughnessPBRGBuffer material = decode_material(gbuffer.material);

    MetallicRoughnessPBR brdf;
    brdf.base_color = gbuffer.base_color.rgb;
    brdf.metallic = material.metallic;
    brdf.perceptual_roughness = material.perceptual_roughness;
    brdf.occlusion = material.occlusion;

    return brdf;
}

const float METALLIC_ROUGHNESS_MIN_REFLECTIVITY = 0.04;

float one_minus_reflectivity(MetallicRoughnessPBR brdf) {
    return (1.0 - METALLIC_ROUGHNESS_MIN_REFLECTIVITY) * (1.0 - brdf.metallic);
}

vec3 get_reflection(MetallicRoughnessPBR brdf) {
    // similar to
    // URP(https://docs.unity3d.com/Packages/com.unity.render-pipelines.universal@10.3/manual/index.html)
    // i guess. The reflectivity actually changes for dielectric materials
    // according to their IORs(index of reflection), but most of them are around
    // 0.02-0.05.
    return mix(vec3(METALLIC_ROUGHNESS_MIN_REFLECTIVITY),
               brdf.base_color,
               brdf.metallic);
}

// f = DGF / (4 (n.v) (n.l)) = DVF
// n, v, l all points from the shading point.
vec3 specular_BRDF(MetallicRoughnessPBR brdf, vec3 n, vec3 v, vec3 l) {
    vec3 h = normalize(v + l);

    float NoV = clamp(abs(dot(n, v)), 0.0, 1.0);
    float NoL = clamp(dot(n, l), 0.0, 1.0);
    float NoH = clamp(dot(n, h), 0.0, 1.0);
    float LoH = clamp(dot(l, h), 0.0, 1.0);

    vec3 f0 = get_reflection(brdf);

    float roughness = brdf.perceptual_roughness * brdf.perceptual_roughness;
    float D = D_GGX(NoH, roughness);
    vec3 F = F_Schlick(LoH, f0);
    float V = V_SmithGGX(NoV, NoL, roughness);

    // remove NAN
    return max(D * V * F, vec3(0.0));
}

vec3 diffuse_BRDF(MetallicRoughnessPBR brdf) {
    return brdf.base_color * one_minus_reflectivity(brdf) / PI;
}

vec3 environment_BRDF(MetallicRoughnessPBR brdf, sampler2D lut_tex, float NoV) {
    // sample pre integrated BRDF lut
    float roughness = brdf.perceptual_roughness * brdf.perceptual_roughness;
    vec2 env_brdf_factor = texture(lut_tex, vec2(roughness, NoV)).xy;
    vec3 f0 = get_reflection(brdf);
    return f0 * env_brdf_factor.x + env_brdf_factor.y;
}

// Importance sampling D(h) (n.h)
//
// most of noise comes from GGX sampling
// importance sampling other terms (like cosine sampling hemisphere) does not
// help much
//
// u is a uniform random variable in [0, 1)
float sample_NoH_from_D(float roughness, float u) {
    // v in range (0, 1]
    float v = 1.0 - u;
    // the denorm will not be 0.0
    return sqrt(v / (v + square(roughness) * (1.0 - v)));
}

// Importance sampling h from D(h) (n.h) with measure dh.
// The macro normal n is (0, 1, 0)
vec3 sample_h_from_D(float roughness, vec2 u) {
    float NoH = sample_NoH_from_D(roughness, u.x);
    float r = sqrt(max(1.0 - square(NoH), 0.0));
    float phi = 2.0 * PI * u.y;
    float y = NoH;
    float x = cos(phi) * r;
    float z = sin(phi) * r;
    return vec3(x, y, z);
}

float calculate_dldh(vec3 v, vec3 l, vec3 h) {
    return square(length(l + v)) / dot(l, h);
}

// Importance sampling h from D(h) (n.h) with measure dl
// The macro normal n is (0, 1, 0)
float sample_l_from_D(vec2 u, float roughness, vec3 v, out vec3 l) {
    vec3 h = sample_h_from_D(roughness, u);
    l = reflect(-v, h);
    float D = D_GGX(h.y, roughness);
    // pdf in dl
    float pdf = D * h.y * calculate_dldh(v, l, h);
    return pdf;
}

// n, v, h should be pre-nomalized.
vec2 specular_env_BRDF_factor(float roughness, vec3 n, vec3 v, vec3 h) {
    vec3 l = normalize(reflect(-v, h));

    if (dot(n, l) < 0.0) {
        return vec2(0.0);
    }

    float NoV = clamp01(abs(dot(n, v)));
    float NoL = clamp01(dot(n, l));
    float NoH = clamp01(dot(n, h));
    float LoH = clamp01(dot(l, h));

    // the Schlick approximation
    // F_Schlick = f + f0 * (1 - f)
    // f0 is calculated based on the base color and the metallic value
    // we only pre-integrate the term f, thus the pre-computed result depends
    // only on roughness and NoV the contribution of f0 can be added back in the
    // actual env brdf calculation
    float F = pow(1.0 - LoH, 5.0);
    float V = V_SmithGGX(NoV, NoL, roughness);

    // the actual pdf is D (n.h)
    // but D is canceled out
    float dldh = calculate_dldh(v, l, h);
    float pdf = NoH;

    vec2 result = vec2(1 - F, F) * V * NoL;
    result *= dldh / pdf;

    // remove NAN
    return max(result, vec2(0.0));
}

// Fitted cone lobe from Practical Realtime Strategies for Accurate Indirect
// Occlusion, Jimenez, SIGGRAPH 2016
float cone_cosine_fitted(float roughness) {
    return exp2(-3.32193 * roughness * roughness);
}

#endif