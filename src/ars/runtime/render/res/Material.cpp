#include "Material.h"
#include "../IContext.h"
#include "../ITexture.h"
#include <ars/runtime/core/Log.h>
#include <ars/runtime/core/Serde.h>
#include <ars/runtime/core/misc/Visitor.h>

namespace ars::render {
std::shared_ptr<IMaterial>
load_material(IContext *context, Resources *res, const ResData &data) {
    if (!data.is_type<IMaterial>()) {
        ARS_LOG_ERROR("Failed to load material: invalid data type");
        return nullptr;
    }

    MaterialResMeta meta = data.meta;
    auto properties = nlohmann::json::from_bson(data.data);
    MaterialInfo info{};
    info.shading_model = meta.type;
    auto m = context->create_material(info);
    for (auto &prop : m->properties()) {
        auto value = properties[prop.name];
        switch (prop.type) {
        case MaterialPropertyType::Texture: {
            auto tex_path = value.get<std::string>();
            if (!tex_path.empty()) {
                m->set(prop.name, res->load<ITexture>(tex_path));
            }
            break;
        }
        case MaterialPropertyType::Float:
            m->set(prop.name, value.get<float>());
            break;
        case MaterialPropertyType::Float2:
            m->set(prop.name, value.get<glm::vec2>());
            break;
        case MaterialPropertyType::Float3:
            m->set(prop.name, value.get<glm::vec3>());
            break;
        case MaterialPropertyType::Float4:
            m->set(prop.name, value.get<glm::vec4>());
            break;
        }
    }

    return m;
}

nlohmann::json serialize_material(IMaterial *material) {
    auto js = nlohmann::json::object();
    for (auto &prop : material->properties()) {
        auto v = material->get_variant(prop.name);
        if (v.has_value()) {
            ars::visit([&](auto &&p) { js[prop.name] = p; }, *v);
        }
    }
    return js;
}
} // namespace ars::render