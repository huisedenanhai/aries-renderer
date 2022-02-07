#ifndef ARS_SAMPLING_GLSL
#define ARS_SAMPLING_GLSL

// Low-discrepancy sequence for quasi monte carlo intergation
float radical_inverse(float i, float base) {
    float v = 0.0;
    float f = 1.0;
    while (i > 0) {
        f /= base;
        v += f * mod(i, base);
        i = floor(i / base);
    }
    return v;
}

float radical_inverse_2(uint i) {
    i = (i << 16u) | (i >> 16u);
    i = ((i & 0x55555555u) << 1u) | ((i & 0xAAAAAAAAu) >> 1u);
    i = ((i & 0x33333333u) << 2u) | ((i & 0xCCCCCCCCu) >> 2u);
    i = ((i & 0x0F0F0F0Fu) << 4u) | ((i & 0xF0F0F0F0u) >> 4u);
    i = ((i & 0x00FF00FFu) << 8u) | ((i & 0xFF00FF00u) >> 8u);
    return float(i) * 2.3283064365386963e-10; // / 0x100000000
}

// see the paper 'Golden Ratio Sequences for Low-Discrepancy Sampling'
float gloden_ratio_sequence(float seed, uint i) {
    float gloden_ratio = 1.618033988749895;
    return fract(seed + i * gloden_ratio);
}

vec2 hammersly(uint i, uint count) {
    return vec2(float(i) / float(count), radical_inverse_2(i));
}

uint tea(uint N, uint val0, uint val1) {
    uint v0 = val0;
    uint v1 = val1;
    uint s0 = 0;

    for (uint n = 0; n < N; n++) {
        s0 += 0x9e3779b9;
        v0 += ((v1 << 4) + 0xa341316c) ^ (v1 + s0) ^ ((v1 >> 5) + 0xc8013ea4);
        v1 += ((v0 << 4) + 0xad90777d) ^ (v0 + s0) ^ ((v0 >> 5) + 0x7e95761e);
    }

    return v0;
}

// Generate random unsigned int in [0, 2^24)
uint lcg(inout uint seed) {
    uint LCG_A = 1664525u;
    uint LCG_C = 1013904223u;
    seed = (LCG_A * seed + LCG_C);
    return seed & 0x00FFFFFF;
}

// Generate random float in [0, 1)
float rnd(inout uint seed) {
    return float(lcg(seed)) / float(0x01000000);
}

// From Next Generation Post Processing in Call of Duty: Advanced Warfare
float interleaved_gradient_noise(vec2 screen_pos) {
    vec3 magic = vec3(0.06711056, 0.00583715, 52.9829189);
    return fract(magic.z * fract(dot(screen_pos, magic.xy)));
}

#endif