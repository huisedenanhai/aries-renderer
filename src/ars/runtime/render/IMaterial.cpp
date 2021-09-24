#include "IMaterial.h"

namespace ars::render {
IMaterial::Mode IMaterial::alpha_mode() const {
    return _alpha_mode;
}

void IMaterial::set_alpha_mode(IMaterial::Mode alpha_mode) {
    _alpha_mode = alpha_mode;
}

bool IMaterial::double_sided() const {
    return _double_sided;
}

void IMaterial::set_double_sided(bool double_sided) {
    _double_sided = double_sided;
}

const glm::vec4 &IMaterial::base_color_factor() const {
    return _base_color_factor;
}

void IMaterial::set_base_color_factor(const glm::vec4 &base_color_factor) {
    _base_color_factor = base_color_factor;
}

const std::shared_ptr<ITexture> &IMaterial::base_color_tex() const {
    return _base_color_tex;
}

void IMaterial::set_base_color_tex(
    const std::shared_ptr<ITexture> &base_color_tex) {
    _base_color_tex = base_color_tex;
}

float IMaterial::metallic_factor() const {
    return _metallic_factor;
}

void IMaterial::set_metallic_factor(float metallic_factor) {
    _metallic_factor = metallic_factor;
}

float IMaterial::roughness_factor() const {
    return _roughness_factor;
}

void IMaterial::set_roughness_factor(float roughness_factor) {
    _roughness_factor = roughness_factor;
}

const std::shared_ptr<ITexture> &IMaterial::metallic_roughness_tex() const {
    return _metallic_roughness_tex;
}

void IMaterial::set_metallic_roughness_tex(
    const std::shared_ptr<ITexture> &metallic_roughness_tex) {
    _metallic_roughness_tex = metallic_roughness_tex;
}

const std::shared_ptr<ITexture> &IMaterial::normal_tex() const {
    return _normal_tex;
}

void IMaterial::set_normal_tex(const std::shared_ptr<ITexture> &normal_tex) {
    _normal_tex = normal_tex;
}

float IMaterial::normal_scale() const {
    return _normal_scale;
}

void IMaterial::set_normal_scale(float normal_scale) {
    _normal_scale = normal_scale;
}

const std::shared_ptr<ITexture> &IMaterial::occlusion_tex() const {
    return _occlusion_tex;
}

void IMaterial::set_occlusion_tex(
    const std::shared_ptr<ITexture> &occlusion_tex) {
    _occlusion_tex = occlusion_tex;
}

float IMaterial::occlusion_strength() const {
    return _occlusion_strength;
}

void IMaterial::set_occlusion_strength(float occlusion_strength) {
    _occlusion_strength = occlusion_strength;
}

const std::shared_ptr<ITexture> &IMaterial::emission_tex() const {
    return _emission_tex;
}

void IMaterial::set_emission_tex(
    const std::shared_ptr<ITexture> &emission_tex) {
    _emission_tex = emission_tex;
}

const glm::vec3 &IMaterial::emission_factor() const {
    return _emission_factor;
}

void IMaterial::set_emission_factor(const glm::vec3 &emission_factor) {
    _emission_factor = emission_factor;
}
} // namespace ars::render