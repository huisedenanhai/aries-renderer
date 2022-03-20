#include "Material.h"
#include "Context.h"
#include <ars/runtime/core/Log.h>
#include <ars/runtime/core/misc/Visitor.h>

namespace ars::render::vk {
MaterialPrototypeRegistry::MaterialPrototypeRegistry(Context *context) {
    auto white_tex = context->default_texture(DefaultTexture::White);

    MaterialPrototypeInfo unlit{};
    unlit.name = "Default Unlit";
    unlit.shading_model = MaterialType::Unlit;
    unlit.add_property("color_factor", glm::vec4(1.0f));
    unlit.add_property("color_tex", white_tex);

    _unlit_material_prototype =
        std::make_unique<MaterialPrototype>(context, unlit);

    MaterialPrototypeInfo pbr{};
    pbr.name = "Default Lit Metallic Roughness";
    pbr.shading_model = MaterialType::MetallicRoughnessPBR;
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
                                     MaterialPrototypeInfo info)
    : _info(std::move(info)), _context(context) {
    init_data_block_layout();
}

std::shared_ptr<IMaterial> MaterialPrototype::create_material() {
    return std::make_shared<Material>(this);
}

namespace {
void property_size_and_alignment(MaterialPropertyType type,
                                 uint32_t &size,
                                 uint32_t &alignment) {
    switch (type) {
    case MaterialPropertyType::Texture:
        size = sizeof(uint32_t);
        alignment = sizeof(uint32_t);
        break;
    case MaterialPropertyType::Float:
        size = sizeof(float);
        alignment = sizeof(float);
        break;
    case MaterialPropertyType::Float2:
        size = sizeof(float) * 2;
        alignment = sizeof(float) * 2;
        break;
    case MaterialPropertyType::Float3:
        size = sizeof(float) * 3;
        alignment = sizeof(float) * 4;
        break;
    case MaterialPropertyType::Float4:
        size = sizeof(float) * 4;
        alignment = sizeof(float) * 4;
        break;
    }
}
} // namespace

void MaterialPrototype::init_data_block_layout() {
    _property_offsets.reserve(_info.properties.size());

    uint32_t cur_ptr = 0;
    uint32_t max_align = 1;

    auto align_ptr = [](uint32_t ptr, uint32_t align) {
        assert(align > 0);
        return ((ptr + align - 1) / align) * align;
    };

    for (auto &p : _info.properties) {
        uint32_t prop_size, prop_align;
        property_size_and_alignment(p.type, prop_size, prop_align);
        cur_ptr = align_ptr(cur_ptr, prop_align);
        max_align = std::max(max_align, prop_align);
        _property_offsets.push_back(cur_ptr);
        cur_ptr += prop_size;
    }
    cur_ptr = align_ptr(cur_ptr, max_align);

    _data_block_size = cur_ptr;
}

uint32_t MaterialPrototype::property_offset(uint32_t index) const {
    assert(index < _property_offsets.size());
    return _property_offsets[index];
}

uint32_t MaterialPrototype::data_block_size() const {
    return _data_block_size;
}

MaterialPrototypeInfo MaterialPrototype::info() {
    return _info;
}

Material::Material(MaterialPrototype *prototype) : _prototype(prototype) {
    auto info = prototype->info();
    _texture_owners.resize(info.properties.size());
    _data_block.resize(prototype->data_block_size());

    // Init with default value
    for (int i = 0; i < info.properties.size(); i++) {
        set_variant_by_index(i, info.properties[i].default_value);
    }
}

void Material::set_variant(const std::string &name,
                           const MaterialPropertyVariant &value) {
    auto info = _prototype->info();
    auto index = info.find_property_index(name);
    if (!index.has_value()) {
        ARS_LOG_ERROR(
            "Material \"{}\" does not have property \"{}\"", info.name, name);
        return;
    }

    set_variant_by_index(*index, value);
}

std::optional<MaterialPropertyVariant>
Material::get_variant(const std::string &name) {
    auto proto = prototype_vk();
    auto info = proto->info();
    auto index = info.find_property_index(name);
    if (!index.has_value()) {
        return std::nullopt;
    }

    assert(*index < info.properties.size());

    auto &p = info.properties[*index];
    auto data_ptr = &_data_block[proto->property_offset(*index)];
    switch (p.type) {
    case MaterialPropertyType::Texture: {
        auto tex = _texture_owners[*index];
        if (tex == nullptr) {
            if (std::holds_alternative<std::shared_ptr<ITexture>>(
                    p.default_value)) {
                tex = std::get<std::shared_ptr<ITexture>>(p.default_value);
            }
        }
        if (tex == nullptr) {
            tex = proto->context()->default_texture(DefaultTexture::White);
        }
        return tex;
    }
    case MaterialPropertyType::Float:
        return *reinterpret_cast<float *>(data_ptr);
    case MaterialPropertyType::Float2:
        return *reinterpret_cast<glm::vec2 *>(data_ptr);
    case MaterialPropertyType::Float3:
        return *reinterpret_cast<glm::vec3 *>(data_ptr);
    case MaterialPropertyType::Float4:
        return *reinterpret_cast<glm::vec4 *>(data_ptr);
    }

    return std::nullopt;
}

MaterialPrototype *upcast(IMaterialPrototype *prototype) {
    return dynamic_cast<MaterialPrototype *>(prototype);
}

MaterialPrototype *Material::prototype_vk() const {
    return _prototype;
}

void Material::set_variant_by_index(uint32_t index,
                                    const MaterialPropertyVariant &value) {
    auto proto = prototype_vk();
    auto info = proto->info();
    assert(index < info.properties.size());

    auto &p = info.properties[index];
    if (value.type() != p.type) {
        ARS_LOG_ERROR("Property \"{}\" of material \"{}\" type mismatch",
                      p.name,
                      info.name);
        return;
    }

    auto data_ptr = &_data_block[proto->property_offset(index)];
    std::visit(ars::make_visitor([&](auto &&v) {
                   using T =
                       std::remove_cv_t<std::remove_reference_t<decltype(v)>>;
                   if constexpr (std::is_same_v<T, std::shared_ptr<ITexture>>) {
                       _texture_owners[index] = v;
                   } else {
                       *reinterpret_cast<T *>(data_ptr) = v;
                   }
               }),
               value);
}

IMaterialPrototype *Material::prototype() {
    return prototype_vk();
}

std::shared_ptr<Material> upcast(const std::shared_ptr<IMaterial> &m) {
    return std::dynamic_pointer_cast<Material>(m);
}
} // namespace ars::render::vk