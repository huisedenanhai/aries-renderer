#include "Material.h"
#include "../IContext.h"
#include "../ITexture.h"
#include <ars/runtime/core/Log.h>
#include <ars/runtime/core/Serde.h>

namespace ars::render {
std::shared_ptr<IMaterial>
load_material(IContext *context, Resources *res, const ResData &data) {
    if (!data.is_type<IMaterial>()) {
        ARS_LOG_ERROR("Failed to load material: invalid data type");
        return nullptr;
    }

    MaterialResMeta meta = data.meta;
    auto properties = nlohmann::json::from_bson(data.data);
    auto proto = context->material_prototype(meta.type);
    auto m = proto->create_material();
    for (auto &prop : proto->properties()) {
        auto value = properties[prop.name];
        switch (prop.type) {
        case MaterialPropertyType::Texture: {
            auto tex_path = value.get<std::string>();
            if (!tex_path.empty()) {
                m->set(prop.id, res->load<ITexture>(tex_path));
            }
            break;
        }
        case MaterialPropertyType::Int:
            m->set(prop.id, value.get<int>());
            break;
        case MaterialPropertyType::Float:
            m->set(prop.id, value.get<float>());
            break;
        case MaterialPropertyType::Float2:
            m->set(prop.id, value.get<glm::vec2>());
            break;
        case MaterialPropertyType::Float3:
            m->set(prop.id, value.get<glm::vec3>());
            break;
        case MaterialPropertyType::Float4:
            m->set(prop.id, value.get<glm::vec4>());
            break;
        }
    }

    return m;
}

nlohmann::json serialize_material(IMaterial *material) {
    auto js = nlohmann::json::object();
    auto proto = material->prototype();
    for (auto &prop : proto->properties()) {
        auto v = material->get_variant(prop.id);
        if (v.has_value()) {
            std::visit([&](auto &&p) { js[prop.name] = p; }, *v);
        }
    }
    return js;
}
} // namespace ars::render