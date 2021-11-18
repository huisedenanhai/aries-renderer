#pragma once

#include "math/AABB.h"
#include "math/Transform.h"
#include <nlohmann/json.hpp>

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
            {"min", v.max},
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
} // namespace nlohmann
