#pragma once

#include "math/AABB.h"
#include "math/Transform.h"
#include <nlohmann/json.hpp>
#include <rttr/type>

namespace nlohmann {
template <typename T> nlohmann::json as_json_array_like(const T &v, int count) {
    auto js = nlohmann::json::array();
    for (int i = 0; i < count; i++) {
        js.push_back(v[i]);
    }
    return js;
}

template <typename VT, typename T>
void from_json_array_number(const nlohmann::json &js, T &v, int count) {
    if (!js.is_array()) {
        return;
    }
    count = std::min(static_cast<int>(js.size()), count);
    for (int i = 0; i < count; i++) {
        auto vi = js[i];
        if (vi.is_number()) {
            v[i] = static_cast<VT>(vi.get<double>());
        }
    }
}

template <typename T, int N> struct adl_serializer<glm::vec<N, T>> {
    using Vec = glm::vec<N, T>;
    static void to_json(json &js, const Vec &v) {
        js = as_json_array_like(v, N);
    }

    static void from_json(const json &js, Vec &v) {
        from_json_array_number<T>(js, v, N);
    }
};

template <typename T> struct adl_serializer<glm::qua<T>> {
    using Quat = glm::qua<T>;
    static void to_json(json &js, const Quat &v) {
        js = as_json_array_like(v, 4);
    }

    static void from_json(const json &js, Quat &v) {
        from_json_array_number<T>(js, v, 4);
    }
};

template <typename T> struct adl_serializer<ars::math::AABB<T>> {
    using AABB = ars::math::AABB<T>;
    static void to_json(json &js, const AABB &v) {
        js = json{
            {"min", v.min},
            {"max", v.max},
        };
    }

    static void from_json(const json &js, AABB &v) {
        js["min"].get_to(v.min);
        js["max"].get_to(v.max);
    }
};

template <typename T> struct adl_serializer<ars::math::XformTRS<T>> {
    using Xform = ars::math::XformTRS<T>;
    static void to_json(json &js, const Xform &v) {
        js = json{
            {"T", v.translation()},
            {"R", v.rotation()},
            {"S", v.scale()},
        };
    }

    static void from_json(const json &js, Xform &v) {
        auto t = v.translation();
        auto r = v.rotation();
        auto s = v.scale();
        js["T"].get_to(t);
        js["R"].get_to(r);
        js["S"].get_to(s);
        v.set_translation(t);
        v.set_rotation(r);
        v.set_scale(s);
    }
};

template <> struct adl_serializer<rttr::instance> {
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

    static void to_json(json &js, const rttr::instance &v) {
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

    static void from_json(const json &js, rttr::instance &v) {
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
};
} // namespace nlohmann
