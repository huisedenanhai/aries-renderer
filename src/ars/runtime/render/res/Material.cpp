#include "Material.h"
#include "../ITexture.h"
#include <ars/runtime/core/Log.h>
#include <ars/runtime/core/Serde.h>

namespace ars::render {
std::shared_ptr<IMaterial> load_material(IContext *context,
                                         const ResData &data) {
    if (data.ty != RES_TYPE_NAME_MATERIAL) {
        ARS_LOG_ERROR("Failed to load material: invalid data type");
        return nullptr;
    }

    return nullptr;
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