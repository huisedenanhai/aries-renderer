#include "Serde.h"

namespace nlohmann {
namespace {
using SerializerRegistry =
    std::map<rttr::type, std::function<json(const rttr::variant &)>>;

using DeserializerRegistry =
    std::map<rttr::type, std::function<void(const json &, rttr::variant &)>>;

template <typename T> static void register_type(SerializerRegistry &reg) {
    reg[rttr::type::get<T>()] = [](const rttr::variant &v) {
        auto ty = v.get_type();
        if (ty.is_wrapper()) {
            return v.template get_wrapped_value<T>();
        }
        return v.template get_value<T>();
    };
}

template <typename T> static void register_type(DeserializerRegistry &reg) {
    reg[rttr::type::get<T>()] = [](const json &js, rttr::variant &v) {
        auto ty = v.get_type();
        if (ty.is_wrapper()) {
            auto &value = const_cast<T &>(v.template get_wrapped_value<T>());
            js.get_to(value);
            return;
        }
        js.get_to(v.template get_value<T>());
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

void adl_serializer<rttr::variant>::to_json(json &js, const rttr::variant &v) {
    static auto serializers = register_callbacks<SerializerRegistry>();
    auto ty = v.get_type();
    if (ty.is_wrapper()) {
        ty = ty.get_wrapped_type();
    }
    auto ser_it = serializers.find(ty);
    if (ser_it != serializers.end()) {
        js = ser_it->second(v);
        return;
    }
    if (v.is_sequential_container()) {
        auto js_arr = nlohmann::json::array();
        auto view = v.create_sequential_view();
        for (const auto &item : view) {
            js_arr.push_back(item);
        }
        js = js_arr;
        return;
    }
    js = json::object();
    auto inst = rttr::instance(v);
    ty = inst.get_derived_type();
    for (auto &prop : ty.get_properties()) {
        auto prop_name = prop.get_name().to_string();
        js[prop_name] = prop.get_value(inst);
    }
}

void adl_serializer<rttr::variant>::from_json(const json &js,
                                              rttr::variant &v) {
    static auto deserializers = register_callbacks<DeserializerRegistry>();
    auto ty = v.get_type();
    if (ty.is_wrapper()) {
        ty = ty.get_wrapped_type();
    }
    auto de_it = deserializers.find(ty);
    if (de_it != deserializers.end()) {
        de_it->second(js, v);
        return;
    }
    auto inst = rttr::instance(v);
    ty = inst.get_derived_type();
    for (auto &prop : ty.get_properties()) {
        auto key = prop.get_name().to_string();
        if (js.find(key) == js.end()) {
            continue;
        }
        auto prop_v = prop.get_value(inst);
        js[key].get_to(prop_v);
        prop.set_value(inst, prop_v);
    }
}
} // namespace nlohmann

namespace ars {
void to_json(nlohmann::json &js, const std::shared_ptr<IRes> &res) {
    js = res->path();
}
} // namespace ars