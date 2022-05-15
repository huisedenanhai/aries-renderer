struct Material {
    vec4 base_color_factor;
    float metallic_factor;
    float roughness_factor;
    float normal_scale;
    float occlusion_strength;
    vec3 emission_factor;
    uint base_color_tex;
    uint metallic_roughness_tex;
    uint normal_tex;
    uint occlusion_tex;
    uint emission_tex;

#ifdef ARS_MATERIAL_ALPHA_CLIP
    float alpha_cutoff;
#endif
};

vec3 material_get_shading_normal_ts(Material m, SurfaceAttribute attr) {
    vec3 normal = sample_tex_2d(m.normal_tex, attr.uv).xyz * 2.0 - 1.0;
    return normal * vec3(m.normal_scale, m.normal_scale, 1.0);
}

// The result may be not normalized
vec3 material_get_shading_normal_vs(Material m, SurfaceAttribute attr) {
    vec3 normal_ts = material_get_shading_normal_ts(m, attr);
    // avoid NAN when tangent is not present
    vec3 shading_normal_vs = normal_ts.x * safe_normalize(attr.tangent) +
                             normal_ts.y * safe_normalize(attr.bitangent) +
                             normal_ts.z * safe_normalize(attr.normal);
    return attr.front_facing ? shading_normal_vs : -shading_normal_vs;
}

float material_get_occlusion(Material m, SurfaceAttribute attr) {
    float occlusion = sample_tex_2d(m.occlusion_tex, attr.uv).r;
    return mix(1.0, occlusion, m.occlusion_strength);
}

vec3 material_get_emission(Material m, SurfaceAttribute attr) {
    return sample_tex_2d(m.emission_tex, attr.uv).rgb * m.emission_factor;
}

struct Closure {
    MetallicRoughnessPBR pbr;
    float alpha;
    vec3 emission;
    vec3 shading_normal;
};

bool material_get_closure(Material m, SurfaceAttribute attr, out Closure c) {
    vec4 base_color =
        sample_tex_2d(m.base_color_tex, attr.uv) * m.base_color_factor;

#ifdef ARS_MATERIAL_ALPHA_CLIP
    if (base_color.a < m.alpha_cutoff) {
        return false;
    }
#endif

    c.pbr.base_color = base_color.rgb;
    c.pbr.occlusion = material_get_occlusion(m, attr);

    vec4 metallic_roughness = sample_tex_2d(m.metallic_roughness_tex, attr.uv);
    c.pbr.metallic = metallic_roughness.b * m.metallic_factor;
    c.pbr.perceptual_roughness = metallic_roughness.g * m.roughness_factor;

    c.alpha = base_color.a;
    c.shading_normal = normalize(material_get_shading_normal_vs(m, attr));
    c.emission = material_get_emission(m, attr);

    return true;
}

void closure_get_gbuffer(Closure c, out GBuffer g) {
    g.base_color = vec4(c.pbr.base_color, c.alpha);
    g.normal_vs = c.shading_normal;
    g.material = encode_material_gbuffer(c.pbr);
    g.shading_model = SHADING_MODEL_METALLIC_ROUGHNESS_PBR;
    g.emission = c.emission;
}