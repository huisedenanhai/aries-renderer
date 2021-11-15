#include "ImGui.h"
#include <glm/gtc/quaternion.hpp>

namespace ars::gui {
bool input_text(const char *label, std::string &str) {
    constexpr size_t BUF_SIZE = 512;
    char buf[BUF_SIZE]{};
    std::memcpy(buf, str.c_str(), std::min(str.size() + 1, BUF_SIZE));
    auto changed = ImGui::InputText(label, buf, std::size(buf));
    // always append an end zero
    buf[BUF_SIZE - 1] = 0;
    if (changed) {
        str = buf;
    }
    return changed;
}

bool input_vec2(const char *label, glm::vec2 &v) {
    return ImGui::InputFloat2(label, &v[0]);
}

bool input_vec3(const char *label, glm::vec3 &v) {
    return ImGui::InputFloat3(label, &v[0]);
}

bool input_vec4(const char *label, glm::vec4 &v) {
    return ImGui::InputFloat4(label, &v[0]);
}

bool input_color3(const char *label, glm::vec3 &v) {
    return ImGui::ColorEdit3(label, &v[0]);
}

bool input_color4(const char *label, glm::vec4 &v) {
    return ImGui::ColorEdit4(label, &v[0]);
}

bool input_rotation(const char *label, glm::quat &q) {
    auto euler_deg = glm::degrees(glm::eulerAngles(q));
    if (ImGui::InputFloat3(label, &euler_deg[0])) {
        q = glm::radians(euler_deg);
        return true;
    }
    return false;
}

bool input_xform(const char *label, math::XformTRS<float> &xform) {
    ImGui::Text("%s", label);
    ImGui::SameLine();
    bool dirty = false;
    int id = 0;
    auto group = [&](auto &&func) {
        ImGui::PushID(id++);
        func();
        ImGui::PopID();
    };

    group([&]() {
        if (ImGui::Button("Reset")) {
            xform = {};
            dirty = true;
        }
    });
    group([&]() {
        auto t = xform.translation();
        if (input_vec3("T", t)) {
            xform.set_translation(t);
            dirty = true;
        }

        ImGui::SameLine();
        if (ImGui::Button("Reset")) {
            xform.set_translation({});
            dirty = true;
        }
    });
    group([&]() {
        auto r = xform.rotation();
        if (input_rotation("R", r)) {
            xform.set_rotation(r);
            dirty = true;
        }
        ImGui::SameLine();
        if (ImGui::Button("Reset")) {
            xform.set_rotation({1.0f, 0.0f, 0.0f, 0.0f});
            dirty = true;
        }
    });
    group([&]() {
        auto s = xform.scale();
        if (input_vec3("S", s)) {
            xform.set_scale(s);
            dirty = true;
        }
        ImGui::SameLine();
        if (ImGui::Button("Reset")) {
            xform.set_scale({1.0f, 1.0f, 1.0f});
            dirty = true;
        }
    });
    return dirty;
}

bool input_instance(rttr::instance instance) {
    auto ty = instance.get_derived_type();
    bool changed = false;

    for (auto &prop : ty.get_properties()) {
        changed = input_property(instance, prop) || changed;
    }

    return changed;
}

template <typename T, typename Func>
bool input_property_type(rttr::instance instance,
                         rttr::property property,
                         bool &changed,
                         Func &&func) {
    if (property.get_type() != rttr::type::get<T>()) {
        return false;
    }

    auto name = property.get_name().to_string();
    auto value = property.get_value(instance).get_value<T>();
    changed = func(name.c_str(), value);
    if (changed) {
        property.set_value(instance, value);
    }
    return true;
}

bool input_property(rttr::instance instance, rttr::property property) {
    bool changed = false;
    if (input_property_type<std::string>(
            instance, property, changed, input_text)) {
        return changed;
    }
    if (input_property_type<float>(
            instance, property, changed, [&](const char *label, float &v) {
                return ImGui::InputFloat(label, &v);
            })) {
        return changed;
    }
    if (input_property_type<int>(
            instance, property, changed, [&](const char *label, int &v) {
                return ImGui::InputInt(label, &v);
            })) {
        return changed;
    }
    if (input_property_type<glm::vec2>(
            instance, property, changed, input_vec2)) {
        return changed;
    }

    std::optional<PropertyDisplay> display{};
    auto display_attr = property.get_metadata(PropertyAttribute::Display);
    if (display_attr.is_valid() &&
        display_attr.can_convert<PropertyDisplay>()) {
        display = display_attr.get_value<PropertyDisplay>();
    }

    if (input_property_type<glm::vec3>(
            instance,
            property,
            changed,
            display == PropertyDisplay::Color ? input_color3 : input_vec3)) {
        return changed;
    }
    if (input_property_type<glm::vec4>(
            instance,
            property,
            changed,
            display == PropertyDisplay::Color ? input_color4 : input_vec4)) {
        return changed;
    }
    if (input_property_type<glm::quat>(
            instance, property, changed, input_rotation)) {
        return changed;
    }
    if (input_property_type<math::XformTRS<float>>(
            instance, property, changed, input_xform)) {
        return changed;
    }

    return false;
}
} // namespace ars::gui
