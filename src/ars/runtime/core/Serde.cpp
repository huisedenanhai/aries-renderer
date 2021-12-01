#include "Serde.h"
#include "Log.h"
#include "ResData.h"

namespace nlohmann {
namespace {
using SerializerRegistry =
    std::map<rttr::type, std::function<json(const rttr::variant &)>>;

using DeserializerRegistry =
    std::map<rttr::type, std::function<void(const json &, rttr::variant &)>>;

rttr::type try_unwrap(const rttr::type &ty) {
    // Don't unwrap res pointer
    if (rttr::type::get<std::shared_ptr<ars::IRes>>() == ty) {
        return ty;
    }
    if (ty.is_wrapper()) {
        return ty.get_wrapped_type();
    }
    return ty;
}

template <typename T> static void register_type(SerializerRegistry &reg) {
    reg[rttr::type::get<T>()] = [](const rttr::variant &v) {
        auto ty = v.get_type();
        if (ty != rttr::type::get<T>() && ty.is_wrapper()) {
            return v.template get_wrapped_value<T>();
        }
        return v.template get_value<T>();
    };
}

template <typename T> static void register_type(DeserializerRegistry &reg) {
    reg[rttr::type::get<T>()] = [](const json &js, rttr::variant &v) {
        auto ty = v.get_type();
        if (ty != rttr::type::get<T>() && ty.is_wrapper()) {
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
    register_type<std::shared_ptr<ars::IRes>>(reg);

    return reg;
}

} // namespace

void adl_serializer<rttr::variant>::to_json(json &js, const rttr::variant &v) {
    static auto serializers = register_callbacks<SerializerRegistry>();
    auto ty = try_unwrap(v.get_type());

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
    ty = try_unwrap(inst.get_derived_type());
    for (auto &prop : ty.get_properties()) {
        auto prop_name = prop.get_name().to_string();
        js[prop_name] = prop.get_value(inst);
    }
}

void adl_serializer<rttr::variant>::from_json(const json &js,
                                              rttr::variant &v) {
    static auto deserializers = register_callbacks<DeserializerRegistry>();
    auto ty = try_unwrap(v.get_type());

    auto de_it = deserializers.find(ty);
    if (de_it != deserializers.end()) {
        de_it->second(js, v);
        return;
    }
    if (v.is_sequential_container() && js.is_array()) {
        auto size = js.size();
        auto view = v.create_sequential_view();
        if (view.is_dynamic()) {
            view.set_size(size);
        }
        size = std::min(js.size(), view.get_size());
        for (int i = 0; i < size; i++) {
            auto item = view.get_value(i);
            js[i].get_to(item);
            view.set_value(i, item);
        }
        return;
    }

    auto inst = rttr::instance(v);
    ty = try_unwrap(inst.get_derived_type());
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
static Resources *s_resources = nullptr;

void set_serde_res_provider(Resources *res) {
    s_resources = res;
}

void to_json(nlohmann::json &js, const std::shared_ptr<IRes> &res) {
    if (res == nullptr) {
        js = "";
        return;
    }
    js = res->path();
}

void from_json(const nlohmann::json &js, std::shared_ptr<IRes> &res) {
    if (s_resources == nullptr) {
        ARS_LOG_ERROR("No Resources provider set for serde, please call "
                      "set_serde_res_provider first, skip deserialize json {} "
                      "for resources",
                      js.dump());
        res = nullptr;
        return;
    }
    if (!js.is_string()) {
        ARS_LOG_ERROR("Invalid resources path, skip deserialize json {} "
                      "for resources",
                      js.dump());
        res = nullptr;
        return;
    }
    res = s_resources->load_res(js.get<std::string>());
}
} // namespace ars