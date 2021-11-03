#ifndef ARS_METALLIC_ROUGHNESS_PBR
#define ARS_METALLIC_ROUGHNESS_PBR

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
};

float METALLIC_ROUGHNESS_MIN_REFLECTIVITY = 0.04;

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

// vec3 environment_BRDF(MetallicRoughnessPBR brdf, float NoV) {
//     // sample pre integrated BRDF lut
//     float roughness = brdf.perceptual_roughness * brdf.perceptual_roughness;
//     vec2 env_brdf_factor = texture(lut_tex, vec2(roughness, NoV)).xy;
//     vec3 f0 = get_reflection(brdf);
//     return f0 * env_brdf_factor.x + env_brdf_factor.y;
// }

#endif