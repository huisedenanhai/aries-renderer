#include "IMaterial.h"
#include <ars/runtime/core/Log.h>
#include <sstream>

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
                std::stringstream ss;
                ss << "material property type mismatch for id " << id;
                log_warn(ss.str());
                return;
            }
            prop.setter(this, value);
            return;
        }
    }
    std::stringstream ss;
    ss << "material property not found with id " << id;
    log_warn(ss.str());
}

std::optional<MaterialPropertyVariant> IMaterial::get_variant(int id) {
    assert(_prototype != nullptr);
    for (auto &prop : _prototype->properties()) {
        if (prop.id == id) {
            return prop.getter(this);
        }
    }
    std::stringstream ss;
    ss << "material property not found with id " << id;
    log_warn(ss.str());
    return std::nullopt;
}

MaterialPropertyInfo::MaterialPropertyInfo(const char *name,
                                           MaterialPropertyType type)
    : name(name), id(IMaterial::str_to_id(name)), type(type) {}

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
    if (std::holds_alternative<std::shared_ptr<ITexture>>(*this)) {
        return MaterialPropertyType::Texture;
    }
    if (std::holds_alternative<int>(*this)) {
        return MaterialPropertyType::Int;
    }
    if (std::holds_alternative<float>(*this)) {
        return MaterialPropertyType::Float;
    }
    if (std::holds_alternative<glm::vec2>(*this)) {
        return MaterialPropertyType::Float2;
    }
    if (std::holds_alternative<glm::vec3>(*this)) {
        return MaterialPropertyType::Float3;
    }
    return MaterialPropertyType::Float4;
}
} // namespace ars::render