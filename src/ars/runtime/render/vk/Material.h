#pragma once

#include "../IMaterial.h"

namespace ars::render::vk {
class Context;

class MetallicRoughnessMaterialPrototype : public IMaterialPrototype {
  public:
    explicit MetallicRoughnessMaterialPrototype(Context *context);

    std::shared_ptr<IMaterial> create_material() override;

  private:
    std::vector<MaterialPropertyInfo> get_property_infos();

    Context *_context = nullptr;
};

class UnlitMaterialPrototype : public IMaterialPrototype {
  public:
    explicit UnlitMaterialPrototype(Context *context);

    std::shared_ptr<IMaterial> create_material() override;

  private:
    std::vector<MaterialPropertyInfo> get_property_infos();

    Context *_context = nullptr;
};

class ErrorMaterialPrototype : public IMaterialPrototype {
  public:
    explicit ErrorMaterialPrototype(Context *context);

    std::shared_ptr<IMaterial> create_material() override;

  private:
    Context *_context = nullptr;
};

class MaterialPrototypeRegistry {
  public:
    explicit MaterialPrototypeRegistry(Context *context);
    ~MaterialPrototypeRegistry() = default;

    IMaterialPrototype *prototype(MaterialType type) const;

  private:
    std::unique_ptr<UnlitMaterialPrototype> _unlit_material_prototype{};
    std::unique_ptr<MetallicRoughnessMaterialPrototype>
        _metallic_roughness_material_prototype{};
    std::unique_ptr<ErrorMaterialPrototype> _error_material_prototype{};
};
} // namespace ars::render::vk