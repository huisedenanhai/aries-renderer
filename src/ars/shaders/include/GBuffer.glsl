#ifndef ARS_GBUFFER_GLSL
#define ARS_GBUFFER_GLSL

struct GBuffer {
    vec4 base_color;
    vec3 normal;
    vec3 material;
    int shading_model;
    vec3 emission;
};

vec3 decode_normal(vec4 raw) {
    return raw.rgb * 2.0 - vec3(1.0);
}

vec4 encode_normal(vec3 normal) {
    return vec4(normal * 0.5 + vec3(0.5), 1.0);
}

int decode_shading_model(float sm) {
    return clamp(int(sm * 255), 0, 255);
}

float encode_shading_model(int sm) {
    return clamp(sm / 255.0, 0.0, 1.0);
}

GBuffer
decode_gbuffer(vec4 gbuffer0, vec4 gbuffer1, vec4 gbuffer2, vec4 gbuffer3) {
    GBuffer res;
    res.base_color = gbuffer0;
    res.normal = decode_normal(gbuffer1);
    res.material = gbuffer2.rgb;
    res.shading_model = decode_shading_model(gbuffer2.a);
    res.emission = gbuffer3.rgb;

    return res;
}

void encode_gbuffer(GBuffer g,
                    out vec4 gbuffer0,
                    out vec4 gbuffer1,
                    out vec4 gbuffer2,
                    out vec4 gbuffer3) {
    gbuffer0 = g.base_color;
    gbuffer1 = encode_normal(g.normal);
    gbuffer2 = vec4(g.material, encode_shading_model(g.shading_model));
    gbuffer3 = vec4(g.emission, 1.0);
}

#endif
