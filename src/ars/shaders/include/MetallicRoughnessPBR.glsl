#ifndef ARS_METALLIC_ROUGHNESS_PBR
#define ARS_METALLIC_ROUGHNESS_PBR

struct MetallicRoughnessPBRParam {
    float occlusion;
    float metallic;
    float perceptual_roughness;
};

vec3 encode_material(MetallicRoughnessPBRParam p) {
    return vec3(p.occlusion, p.metallic, p.perceptual_roughness);
}

MetallicRoughnessPBRParam decode_material(vec3 m) {
    MetallicRoughnessPBRParam p;
    p.occlusion = m.r;
    p.metallic = m.g;
    p.perceptual_roughness = m.b;

    return p;
}

#endif