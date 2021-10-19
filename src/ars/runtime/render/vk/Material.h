#pragma once

#include "../IMaterial.h"
#include "Texture.h"
#include "Vulkan.h"

namespace ars::render::vk {
class Context;

struct MaterialTextureHandle {
  public:
    [[nodiscard]] std::shared_ptr<ITexture> texture() const;
    [[nodiscard]] Handle<Texture> vk_texture() const;

    void set_texture(const std::shared_ptr<ITexture> &tex);

  private:
    std::shared_ptr<ITexture> _texture{};
    Handle<Texture> _vk_texture{};
};

template <typename MT> class MaterialPrototype : public IMaterialPrototype {
  public:
    [[maybe_unused]] explicit MaterialPrototype(Context *context)
        : IMaterialPrototype(MT::Type, MT::reflect()), _context(context) {}

    std::shared_ptr<IMaterial> create_material() override {
        return std::make_shared<MT>(this);
    }

    [[nodiscard]] Context *context() const {
        return _context;
    }

  private:
    Context *_context = nullptr;
};

template <typename M, typename T>
MaterialPropertyInfo make_material_property(const char *name, T M::*mem) {
    if constexpr (std::is_same_v<T, MaterialTextureHandle>) {
        return MaterialPropertyInfo(
            name,
            MaterialPropertyType::Texture,
            [mem](IMaterial *m, const MaterialPropertyVariant &v) {
                auto mat = dynamic_cast<M *>(m);
                (mat->*mem).set_texture(std::get<std::shared_ptr<ITexture>>(v));
            },
            [mem](IMaterial *m) {
                auto mat = dynamic_cast<M *>(m);
                return (mat->*mem).texture();
            });
    } else {
        return MaterialPropertyInfo::make(name, mem);
    }
}

class MetallicRoughnessMaterial : public IMaterial {
  public:
    static constexpr MaterialType Type = MaterialType::MetallicRoughnessPBR;
    using Prototype = MaterialPrototype<MetallicRoughnessMaterial>;

    explicit MetallicRoughnessMaterial(Prototype *prototype);
    static std::vector<MaterialPropertyInfo> reflect();

    MaterialAlphaMode alpha_mode = MaterialAlphaMode::Opaque;
    bool double_sided = false;
    glm::vec4 base_color_factor = {1.0f, 1.0f, 1.0f, 1.0f};
    MaterialTextureHandle base_color_tex{};
    float metallic_factor = 1.0f;
    float roughness_factor = 1.0f;
    MaterialTextureHandle metallic_roughness_tex{};
    MaterialTextureHandle normal_tex{};
    float normal_scale = 1.0f;
    MaterialTextureHandle occlusion_tex{};
    float occlusion_strength = 1.0f;
    MaterialTextureHandle emission_tex{};
    glm::vec3 emission_factor = {0.0f, 0.0f, 0.0f};
};

class UnlitMaterial : public IMaterial {
  public:
    static constexpr MaterialType Type = MaterialType::Unlit;
    using Prototype = MaterialPrototype<UnlitMaterial>;

    explicit UnlitMaterial(Prototype *prototype);
    static std::vector<MaterialPropertyInfo> reflect();

    glm::vec4 color_factor = {1.0f, 1.0f, 1.0f, 1.0f};
    MaterialTextureHandle color_tex{};
};

class ErrorMaterial : public IMaterial {
  public:
    static constexpr MaterialType Type = MaterialType::Error;
    using Prototype = MaterialPrototype<ErrorMaterial>;

    explicit ErrorMaterial(Prototype *prototype);
    static std::vector<MaterialPropertyInfo> reflect();
};

class MaterialPrototypeRegistry {
  public:
    explicit MaterialPrototypeRegistry(Context *context);
    ~MaterialPrototypeRegistry() = default;

    [[nodiscard]] IMaterialPrototype *prototype(MaterialType type) const;

  private:
    std::unique_ptr<UnlitMaterial::Prototype> _unlit_material_prototype{};
    std::unique_ptr<MetallicRoughnessMaterial::Prototype>
        _metallic_roughness_material_prototype{};
    std::unique_ptr<ErrorMaterial::Prototype> _error_material_prototype{};
};
} // namespace ars::render::vk