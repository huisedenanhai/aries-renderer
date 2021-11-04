#include "IMaterial.h"
#include <ars/runtime/core/Log.h>

namespace ars::render {
int IMaterial::str_to_id(const std::string &name) {
    return static_cast<int>(std::hash<std::string>()(name));
}

IMaterial::IMaterial(IMaterialPrototype *prototype) : _prototype(prototype) {}

MaterialType IMaterial::type() const {
    return _prototype->type();
}

IMaterialPrototype *IMaterial::prototype() const {
    return _prototype;
}

void IMaterial::set_variant(int id, const MaterialPropertyVariant &value) {
    assert(_prototype != nullptr);
    for (auto &prop : _prototype->properties()) {
        if (prop.id == id) {
            if (value.type() != prop.type) {
                ARS_LOG_WARN("material property type mismatch for id {}", id);
                return;
            }
            prop.setter(this, value);
            return;
        }
    }
    ARS_LOG_WARN("Material property not found with id {}", id);
}

std::optional<MaterialPropertyVariant> IMaterial::get_variant(int id) {
    assert(_prototype != nullptr);
    for (auto &prop : _prototype->properties()) {
        if (prop.id == id) {
            return prop.getter(this);
        }
    }
    ARS_LOG_WARN("Material property not found with id {}", id);
    return std::nullopt;
}

MaterialPropertyInfo::MaterialPropertyInfo(const char *name,
                                           MaterialPropertyType type,
                                           Setter setter,
                                           Getter getter)
    : name(name), id(IMaterial::str_to_id(name)), type(type),
      setter(std::move(setter)), getter(std::move(getter)) {}

MaterialType IMaterialPrototype::type() const {
    return _type;
}

const std::vector<MaterialPropertyInfo> &
IMaterialPrototype::properties() const {
    return _properties;
}

IMaterialPrototype::IMaterialPrototype(
    MaterialType type, std::vector<MaterialPropertyInfo> properties)
    : _type(type), _properties(std::move(properties)) {}

MaterialPropertyType MaterialPropertyVariant::type() const {
    return std::visit(
        [](auto &&v) {
            using T = std::remove_cv_t<std::remove_reference_t<decltype(v)>>;
            return MaterialPropertyTypeTrait<T>::Type;
        },
        *this);
}
} // namespace ars::render