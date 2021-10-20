#include "Material.h"
#include "Context.h"

namespace ars::render::vk {
std::vector<MaterialPropertyInfo> UnlitMaterial::reflect() {
#define ARS_VK_MATERIAL_PROPERTY(name)                                         \
    make_material_property(#name, &UnlitMaterial::name)

    return {
        ARS_VK_MATERIAL_PROPERTY(color_factor),
        ARS_VK_MATERIAL_PROPERTY(color_tex),
    };

#undef ARS_VK_MATERIAL_PROPERTY
}

std::vector<MaterialPropertyInfo> MetallicRoughnessMaterial::reflect() {
#define ARS_VK_MATERIAL_PROPERTY(name)                                         \
    make_material_property(#name, &MetallicRoughnessMaterial::name)

    return {
        ARS_VK_MATERIAL_PROPERTY(alpha_mode),
        ARS_VK_MATERIAL_PROPERTY(double_sided),
        ARS_VK_MATERIAL_PROPERTY(base_color_factor),
        ARS_VK_MATERIAL_PROPERTY(base_color_tex),
        ARS_VK_MATERIAL_PROPERTY(metallic_factor),
        ARS_VK_MATERIAL_PROPERTY(roughness_factor),
        ARS_VK_MATERIAL_PROPERTY(metallic_roughness_tex),
        ARS_VK_MATERIAL_PROPERTY(normal_tex),
        ARS_VK_MATERIAL_PROPERTY(normal_scale),
        ARS_VK_MATERIAL_PROPERTY(occlusion_tex),
        ARS_VK_MATERIAL_PROPERTY(occlusion_strength),
        ARS_VK_MATERIAL_PROPERTY(emission_tex),
        ARS_VK_MATERIAL_PROPERTY(emission_factor),
    };

#undef ARS_VK_MATERIAL_PROPERTY
}

MaterialPrototypeRegistry::MaterialPrototypeRegistry(Context *context) {
    _unlit_material_prototype =
        std::make_unique<UnlitMaterial::Prototype>(context);
    _metallic_roughness_material_prototype =
        std::make_unique<MetallicRoughnessMaterial::Prototype>(context);
    _error_material_prototype =
        std::make_unique<ErrorMaterial::Prototype>(context);
}

IMaterialPrototype *
MaterialPrototypeRegistry::prototype(MaterialType type) const {
    switch (type) {
    case MaterialType::Error:
        return _error_material_prototype.get();
    case MaterialType::Unlit:
        return _unlit_material_prototype.get();
    case MaterialType::MetallicRoughnessPBR:
        return _metallic_roughness_material_prototype.get();
    }
    return _error_material_prototype.get();
}

std::shared_ptr<ITexture> MaterialTextureHandle::texture() const {
    return _texture == nullptr ? _default_texture : _texture;
}

Handle<Texture> MaterialTextureHandle::vk_texture() const {
    return upcast(texture().get());
}

void MaterialTextureHandle::set_texture(const std::shared_ptr<ITexture> &tex) {
    _texture = tex;
}

std::shared_ptr<ITexture> MaterialTextureHandle::default_texture() const {
    return _default_texture;
}

void MaterialTextureHandle::set_default_texture(
    const std::shared_ptr<ITexture> &tex) {
    _default_texture = tex;
}

ErrorMaterial::ErrorMaterial(Prototype *prototype) : IMaterial(prototype) {}

std::vector<MaterialPropertyInfo> ErrorMaterial::reflect() {
    return {};
}

UnlitMaterial::UnlitMaterial(Prototype *prototype) : IMaterial(prototype) {
    auto ctx = prototype->context();
    color_tex.set_default_texture(ctx->default_texture(DefaultTexture::White));
}

MetallicRoughnessMaterial::MetallicRoughnessMaterial(Prototype *prototype)
    : IMaterial(prototype) {
    auto ctx = prototype->context();
    base_color_tex.set_default_texture(
        ctx->default_texture(DefaultTexture::White));
    metallic_roughness_tex.set_default_texture(
        ctx->default_texture(DefaultTexture::White));
    normal_tex.set_default_texture(
        ctx->default_texture(DefaultTexture::Normal));
    occlusion_tex.set_default_texture(
        ctx->default_texture(DefaultTexture::White));
    emission_tex.set_default_texture(
        ctx->default_texture(DefaultTexture::White));
}
} // namespace ars::render::vk