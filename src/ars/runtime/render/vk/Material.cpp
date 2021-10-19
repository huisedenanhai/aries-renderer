#include "Material.h"

namespace ars::render::vk {
std::vector<MaterialPropertyInfo> UnlitMaterialPrototype::get_property_infos() {
    return {
        MaterialPropertyInfo("color_factor", MaterialPropertyType::Float4),
        MaterialPropertyInfo("color_tex", MaterialPropertyType::Texture),
    };
}

UnlitMaterialPrototype::UnlitMaterialPrototype(Context *context)
    : IMaterialPrototype(MaterialType::Unlit, get_property_infos()),
      _context(context) {}

std::shared_ptr<IMaterial> UnlitMaterialPrototype::create_material() {
    return std::shared_ptr<IMaterial>();
}

std::vector<MaterialPropertyInfo>
MetallicRoughnessMaterialPrototype::get_property_infos() {
    return {
        MaterialPropertyInfo("alpha_mode", MaterialPropertyType::Int),
        MaterialPropertyInfo("double_sided", MaterialPropertyType::Int),
        MaterialPropertyInfo("base_color_factor", MaterialPropertyType::Float4),
        MaterialPropertyInfo("base_color_tex", MaterialPropertyType::Texture),
        MaterialPropertyInfo("metallic_factor", MaterialPropertyType::Float),
        MaterialPropertyInfo("roughness_factor", MaterialPropertyType::Float),
        MaterialPropertyInfo("metallic_roughness_tex",
                             MaterialPropertyType::Texture),
        MaterialPropertyInfo("normal_tex", MaterialPropertyType::Texture),
        MaterialPropertyInfo("normal_scale", MaterialPropertyType::Float),
        MaterialPropertyInfo("occlusion_tex", MaterialPropertyType::Texture),
        MaterialPropertyInfo("occlusion_strength", MaterialPropertyType::Float),
        MaterialPropertyInfo("emission_tex", MaterialPropertyType::Texture),
        MaterialPropertyInfo("emission_factor", MaterialPropertyType::Float3),
    };
}

MetallicRoughnessMaterialPrototype::MetallicRoughnessMaterialPrototype(
    Context *context)
    : IMaterialPrototype(MaterialType::MetallicRoughnessPBR,
                        get_property_infos()),
      _context(context) {}

std::shared_ptr<IMaterial>
MetallicRoughnessMaterialPrototype::create_material() {
    return std::shared_ptr<IMaterial>();
}

std::shared_ptr<IMaterial> ErrorMaterialPrototype::create_material() {
    return std::shared_ptr<IMaterial>();
}

ErrorMaterialPrototype::ErrorMaterialPrototype(Context *context)
    : IMaterialPrototype(MaterialType::Error, {}), _context(context) {}

MaterialPrototypeRegistry::MaterialPrototypeRegistry(Context *context) {
    _unlit_material_prototype =
        std::make_unique<UnlitMaterialPrototype>(context);
    _metallic_roughness_material_prototype =
        std::make_unique<MetallicRoughnessMaterialPrototype>(context);
    _error_material_prototype =
        std::make_unique<ErrorMaterialPrototype>(context);
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
} // namespace ars::render::vk