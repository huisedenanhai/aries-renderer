#include "Material.h"
#include "Context.h"
#include <ars/runtime/core/Log.h>
#include <ars/runtime/core/misc/Visitor.h>

namespace ars::render::vk {
MaterialPrototypeRegistry::MaterialPrototypeRegistry(Context *context) {
    auto white_tex = context->default_texture(DefaultTexture::White);

    MaterialPropertyBlockInfo unlit{};
    unlit.name = "Default Unlit";
    unlit.add_property("color_factor", glm::vec4(1.0f));
    unlit.add_property("color_tex", white_tex);

    _unlit_material_prototype =
        std::make_unique<MaterialPrototype>(context, unlit);

    MaterialPropertyBlockInfo pbr{};
    pbr.name = "Default Lit Metallic Roughness";
    pbr.add_property("base_color_factor", glm::vec4(1.0f));
    pbr.add_property("metallic_factor", 1.0f);
    pbr.add_property("roughness_factor", 1.0f);
    pbr.add_property("normal_scale", 1.0f);
    pbr.add_property("occlusion_strength", 1.0f);
    pbr.add_property("emission_factor", glm::vec3(0.0f));
    pbr.add_property("base_color_tex", white_tex);
    pbr.add_property("metallic_roughness_tex", white_tex);
    pbr.add_property("normal_tex",
                     context->default_texture(DefaultTexture::Normal));
    pbr.add_property("occlusion_tex", white_tex);
    pbr.add_property("emission_tex", white_tex);

    _metallic_roughness_material_prototype =
        std::make_unique<MaterialPrototype>(context, pbr);
}

IMaterialPrototype *
MaterialPrototypeRegistry::prototype(MaterialType type) const {
    switch (type) {
    case MaterialType::Unlit:
        return _unlit_material_prototype.get();
    case MaterialType::MetallicRoughnessPBR:
        return _metallic_roughness_material_prototype.get();
    }
    return _metallic_roughness_material_prototype.get();
}

MaterialPrototype::MaterialPrototype(Context *context,
                                     const MaterialPropertyBlockInfo &info)
    : _context(context) {
    _block_layout =
        std::make_shared<MaterialPropertyBlockLayout>(context, info);
}

std::shared_ptr<IMaterial> MaterialPrototype::create_material() {
    return std::make_shared<Material>(this);
}

MaterialPropertyBlockInfo MaterialPrototype::info() {
    return _block_layout->info();
}

Context *MaterialPrototype::context() const {
    return _context;
}

std::shared_ptr<MaterialPropertyBlockLayout>
MaterialPrototype::property_block_layout() const {
    return _block_layout;
}

Material::Material(MaterialPrototype *prototype) : _prototype(prototype) {
    _property_block =
        std::make_unique<MaterialPropertyBlock>(
        _prototype->property_block_layout());
}

void Material::set_variant(const std::string &name,
                           const MaterialPropertyVariant &value) {
    _property_block->set_variant(name, value);
}

std::optional<MaterialPropertyVariant>
Material::get_variant(const std::string &name) {
    return _property_block->get_variant(name);
}

MaterialPrototype *upcast(IMaterialPrototype *prototype) {
    return dynamic_cast<MaterialPrototype *>(prototype);
}

MaterialPrototype *Material::prototype_vk() const {
    return _prototype;
}

IMaterialPrototype *Material::prototype() {
    return prototype_vk();
}

MaterialPropertyBlock *Material::property_block() const {
    return _property_block.get();
}

std::shared_ptr<Material> upcast(const std::shared_ptr<IMaterial> &m) {
    return std::dynamic_pointer_cast<Material>(m);
}
} // namespace ars::render::vk