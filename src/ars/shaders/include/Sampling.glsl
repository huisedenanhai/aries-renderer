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

vec2 hammersly(uint i, uint count) {
    return vec2(float(i) / float(count), radical_inverse_2(i));
}

#endif