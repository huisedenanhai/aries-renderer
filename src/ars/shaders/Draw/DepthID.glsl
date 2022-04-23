#ifdef ARS_UBER_SHADER_DEFINE_MATERIAL

#ifdef ARS_MATERIAL_ALPHA_CLIP

struct Material {
    vec4 base_color_factor;
    uint base_color_tex;
    float alpha_cutoff;
};

#define ARS_MATERIAL_SAMPLER_2D_COUNT 1

#else
#define ARS_EMPTY_MATERIAL
#endif

#endif

#ifdef ARS_UBER_SHADER_DEFINE_FRAG_SHADER

#ifdef ARS_OBJECT_ID_PASS
layout(location = 0) out uint out_id;
#endif

void main() {
#ifdef ARS_MATERIAL_ALPHA_CLIP
    Material m = get_material();
    vec4 base_color =
        sample_tex_2d(m.base_color_tex, in_uv) * m.base_color_factor;

    if (base_color.a < m.alpha_cutoff) {
        discard;
    }
#endif

#ifdef ARS_OBJECT_ID_PASS
    out_id = get_instance().custom_id;
#endif
}
#endif