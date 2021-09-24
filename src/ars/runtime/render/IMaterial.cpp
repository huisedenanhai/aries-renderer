#include "IMaterial.h"

namespace ars::render {
bool IMaterial::double_sided() const {
    return _double_sided;
}

void IMaterial::set_double_sided(bool doubleSided) {
    _double_sided = doubleSided;
}

const glm::vec4 &IMaterial::base_color_factor() const {
    return _base_color_factor;
}

void IMaterial::set_base_color_factor(const glm::vec4 &baseColorFactor) {
    _base_color_factor = baseColorFactor;
}

const std::shared_ptr<ITexture> &IMaterial::base_color_tex() const {
    return _base_color_tex;
}

void IMaterial::set_base_color_tex(
    const std::shared_ptr<ITexture> &baseColorTex) {
    _base_color_tex = baseColorTex;
}

float IMaterial::metallic_factor() const {
    return _metallic_factor;
}

void IMaterial::set_metallic_factor(float metallicFactor) {
    _metallic_factor = metallicFactor;
}

float IMaterial::roughness_factor() const {
    return _roughness_factor;
}

void IMaterial::set_roughness_factor(float roughnessFactor) {
    _roughness_factor = roughnessFactor;
}

const std::shared_ptr<ITexture> &IMaterial::metallic_roughness_tex() const {
    return _metallic_roughness_tex;
}

void IMaterial::set_metallic_roughness_tex(
    const std::shared_ptr<ITexture> &metallicRoughnessTex) {
    _metallic_roughness_tex = metallicRoughnessTex;
}

const std::shared_ptr<ITexture> &IMaterial::normal_tex() const {
    return _normal_tex;
}

void IMaterial::set_normal_tex(const std::shared_ptr<ITexture> &normalTex) {
    _normal_tex = normalTex;
}

float IMaterial::normal_scale() const {
    return _normal_scale;
}

void IMaterial::set_normal_scale(float normalScale) {
    _normal_scale = normalScale;
}

const std::shared_ptr<ITexture> &IMaterial::occlusion_tex() const {
    return _occlusion_tex;
}

void IMaterial::set_occlusion_tex(
    const std::shared_ptr<ITexture> &occlusionTex) {
    _occlusion_tex = occlusionTex;
}

float IMaterial::occlusion_strength() const {
    return _occlusion_strength;
}

void IMaterial::set_occlusion_strength(float occlusionStrength) {
    _occlusion_strength = occlusionStrength;
}

const std::shared_ptr<ITexture> &IMaterial::emission_tex() const {
    return _emission_tex;
}

void IMaterial::set_emission_tex(const std::shared_ptr<ITexture> &emissionTex) {
    _emission_tex = emissionTex;
}

const glm::vec3 &IMaterial::emission_factor() const {
    return _emission_factor;
}

void IMaterial::set_emission_factor(const glm::vec3 &emissionFactor) {
    _emission_factor = emissionFactor;
}

IMaterial::Mode IMaterial::alpha_mode() const {
    return _alpha_mode;
}

void IMaterial::set_alpha_mode(IMaterial::Mode alphaMode) {
    _alpha_mode = alphaMode;
}
} // namespace ars::render