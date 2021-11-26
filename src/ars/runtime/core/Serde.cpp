#include "Serde.h"

namespace nlohmann {
namespace {
using SerializerRegistry = std::map<
    rttr::type,
    std::function<json(const rttr::instance &, const rttr::property &)>>;

using DeserializerRegistry = std::map<
    rttr::type,
    std::function<void(
        const json &, const rttr::instance &, const rttr::property &)>>;

template <typename T> static void register_type(SerializerRegistry &reg) {
    reg[rttr::type::get<T>()] = [](const rttr::instance &ins,
                                   const rttr::property &prop) {
        return prop.get_value(ins).template get_value<T>();
    };
}

template <typename T> static void register_type(DeserializerRegistry &reg) {
    reg[rttr::type::get<T>()] = [](const json &js,
                                   const rttr::instance &ins,
                                   const rttr::property &prop) {
        prop.set_value(ins, js.template get<T>());
    };
}

template <typename Reg> static Reg register_callbacks() {
    Reg reg{};
    register_type<size_t>(reg);
    register_type<uint8_t>(reg);
    register_type<uint16_t>(reg);
    register_type<uint32_t>(reg);
    register_type<uint64_t>(reg);
    register_type<int8_t>(reg);
    register_type<int16_t>(reg);
    register_type<int32_t>(reg);
    register_type<int64_t>(reg);
    register_type<char>(reg);
    register_type<short>(reg);
    register_type<int>(reg);
    register_type<long>(reg);
    register_type<long long>(reg);
    register_type<unsigned char>(reg);
    register_type<unsigned short>(reg);
    register_type<unsigned int>(reg);
    register_type<unsigned long>(reg);
    register_type<unsigned long long>(reg);
    register_type<float>(reg);
    register_type<double>(reg);
    register_type<std::string>(reg);
    register_type<glm::vec2>(reg);
    register_type<glm::vec3>(reg);
    register_type<glm::vec4>(reg);
    register_type<glm::dvec2>(reg);
    register_type<glm::dvec3>(reg);
    register_type<glm::quat>(reg);
    register_type<glm::dquat>(reg);
    register_type<ars::math::XformTRS<float>>(reg);
    register_type<ars::math::XformTRS<double>>(reg);
    register_type<ars::math::AABB<float>>(reg);
    register_type<ars::math::AABB<double>>(reg);

    return reg;
}
} // namespace

void adl_serializer<rttr::instance>::to_json(json &js,
                                             const rttr::instance &v) {
    js = json::object();
    auto ty = v.get_derived_type();
    static auto serializers = register_callbacks<SerializerRegistry>();
    for (auto &prop : ty.get_properties()) {
        auto prop_ty = prop.get_type();
        auto ser_it = serializers.find(prop_ty);
        if (ser_it == serializers.end()) {
            continue;
        }
        js[prop.get_name().to_string()] = ser_it->second(v, prop);
    }
}

void adl_serializer<rttr::instance>::from_json(const json &js,
                                               rttr::instance &v) {
    auto ty = v.get_derived_type();
    static auto deserializers = register_callbacks<DeserializerRegistry>();
    for (auto &prop : ty.get_properties()) {
        auto prop_ty = prop.get_type();
        auto key = prop.get_name().to_string();
        if (js.find(key) == js.end()) {
            continue;
        }
        auto de_it = deserializers.find(prop_ty);
        if (de_it == deserializers.end()) {
            continue;
        }
        de_it->second(js[key], v, prop);
    }
}
} // namespace nlohmann

namespace ars {
void to_json(nlohmann::json &js, const std::shared_ptr<IRes> &res) {
    js = res->path();
}
} // namespace ars