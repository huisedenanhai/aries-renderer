#include "Material.h"
#include "Context.h"
#include <ars/runtime/core/Log.h>
#include <ars/runtime/core/misc/Visitor.h>

namespace ars::render::vk {
MaterialFactory::MaterialFactory(Context *context) : _context(context) {
    init_unlit_template();
    init_metallic_roughness_template();
}

std::shared_ptr<Material> MaterialFactory::create_material(MaterialType type) {
    switch (type) {
    case MaterialType::Unlit:
        return _unlit_template.create();
    case MaterialType::MetallicRoughnessPBR:
        return _metallic_roughness_template.create();
    }
    return _metallic_roughness_template.create();
}

void MaterialFactory::init_unlit_template() {
    auto white_tex = _context->default_texture(DefaultTexture::White);
    MaterialPropertyBlockInfo unlit{};
    unlit.name = "Unlit";
    unlit.add_property("color_factor", glm::vec4(1.0f));
    unlit.add_property("color_tex", white_tex);

    _unlit_template.type = MaterialType::Unlit;
    _unlit_template.property_layout =
        std::make_shared<MaterialPropertyBlockLayout>(_context, unlit);

    auto &geom_pass = _unlit_template.passes[RenderPassID_Geometry];
    geom_pass.property_layout = _unlit_template.property_layout;
    // TODO init pipeline
}

void MaterialFactory::init_metallic_roughness_template() {
    auto white_tex = _context->default_texture(DefaultTexture::White);

    MaterialPropertyBlockInfo pbr{};
    pbr.name = "MetallicRoughnessLit";
    pbr.add_property("base_color_factor", glm::vec4(1.0f));
    pbr.add_property("metallic_factor", 1.0f);
    pbr.add_property("roughness_factor", 1.0f);
    pbr.add_property("normal_scale", 1.0f);
    pbr.add_property("occlusion_strength", 1.0f);
    pbr.add_property("emission_factor", glm::vec3(0.0f));
    pbr.add_property("base_color_tex", white_tex);
    pbr.add_property("metallic_roughness_tex", white_tex);
    pbr.add_property("normal_tex",
                     _context->default_texture(DefaultTexture::Normal));
    pbr.add_property("occlusion_tex", white_tex);
    pbr.add_property("emission_tex", white_tex);

    _metallic_roughness_template.type = MaterialType::MetallicRoughnessPBR;
    _metallic_roughness_template.property_layout =
        std::make_shared<MaterialPropertyBlockLayout>(_context, pbr);

    auto &geom_pass =
        _metallic_roughness_template.passes[RenderPassID_Geometry];
    geom_pass.property_layout = _metallic_roughness_template.property_layout;
    // TODO init pipeline
}

void Material::set_variant(const std::string &name,
                           const MaterialPropertyVariant &value) {
    if (_property_block != nullptr) {
        _property_block->set_variant(name, value);
    }
    // Also set properties with same name for passes
    for (auto &p : _passes) {
        if (p.property_block == nullptr) {
            continue;
        }
        if (p.property_block->layout()
                ->info()
                .find_property_index(name)
                .has_value()) {
            p.property_block->set(name, value);
        }
    }
}

std::optional<MaterialPropertyVariant>
Material::get_variant(const std::string &name) {
    if (_property_block == nullptr) {
        return std::nullopt;
    }
    return _property_block->get_variant(name);
}

Material::Material(
    MaterialType type,
    const std::shared_ptr<MaterialPropertyBlockLayout> &property_layout,
    MaterialPassArray passes)
    : _type(type), _passes(std::move(passes)) {
    if (property_layout != nullptr) {
        _property_block =
            std::make_unique<MaterialPropertyBlock>(property_layout);
    }
}

std::vector<MaterialPropertyInfo> Material::properties() {
    if (_property_block == nullptr) {
        return {};
    }
    return _property_block->layout()->info().properties;
}

MaterialType Material::type() {
    return _type;
}

MaterialPass Material::pass(RenderPassID pass_id) {
    auto id = static_cast<uint32_t>(pass_id);
    assert(id < _passes.size());

    MaterialPass p{};
    p.property_block = _passes[id].property_block.get();
    p.pipeline = _passes[id].pipeline.get();

    return p;
}

std::shared_ptr<Material> upcast(const std::shared_ptr<IMaterial> &m) {
    return std::dynamic_pointer_cast<Material>(m);
}

MaterialPassOwned MaterialPassTemplate::create() {
    MaterialPassOwned p{};

    if (property_layout != nullptr) {
        p.property_block =
            std::make_unique<MaterialPropertyBlock>(property_layout);
    }
    p.pipeline = pipeline;

    return p;
}

std::shared_ptr<Material> MaterialTemplate::create() {
    MaterialPassArray ps{};
    for (int i = 0; i < RenderPassID_Count; i++) {
        ps[i] = passes[i].create();
    }

    return std::make_shared<Material>(type, property_layout, std::move(ps));
}
} // namespace ars::render::vk