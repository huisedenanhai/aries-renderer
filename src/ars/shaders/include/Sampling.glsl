#ifndef ARS_SAMPLEING_GLSL
#define ARS_SAMPLEING_GLSL

// Low-discrepancy sequence for quasi monte carlo intergation
float halton(float i, float base) {
    float v = 0.0;
    float f = 1.0;
    while (i > 0) {
        f /= base;
        v += f * mod(i, base);
        i = floor(i / base);
    }
    return v;
}

vec2 hammersly(float i, float count) {
    return vec2(i / count, halton(i, 2.0));
}

#endif