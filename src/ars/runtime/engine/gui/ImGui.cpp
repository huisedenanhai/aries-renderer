#include "ImGui.h"
#include "../Engine.h"
#include <ars/runtime/core/ResData.h>
#include <glm/gtc/quaternion.hpp>
#include <spdlog/fmt/fmt.h>

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

bool input_mat4(const char *label, glm::mat4 &v) {
    bool changed = false;
    auto t = glm::transpose(v);
    if (ImGui::TreeNode(label)) {
        changed = input_vec4("Row 0", t[0]) || changed;
        changed = input_vec4("Row 1", t[1]) || changed;
        changed = input_vec4("Row 2", t[2]) || changed;
        changed = input_vec4("Row 3", t[3]) || changed;
        ImGui::TreePop();
    }
    if (changed) {
        v = glm::transpose(t);
    }
    return changed;
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

bool input_instance(const rttr::instance &instance) {
    auto inst = instance.get_type().get_raw_type().is_wrapper()
                    ? instance.get_wrapped_instance()
                    : instance;
    auto ty = inst.get_derived_type();
    bool changed = false;

    for (auto &prop : ty.get_properties()) {
        changed = input_property(inst, prop) || changed;
    }

    return changed;
}

bool input_property(const rttr::instance &instance, rttr::property property) {
    std::optional<PropertyDisplay> display{};
    auto display_attr = property.get_metadata(PropertyAttribute::Display);
    if (display_attr.is_valid() &&
        display_attr.can_convert<PropertyDisplay>()) {
        display = display_attr.get_value<PropertyDisplay>();
    }

    if (display.has_value() && display.value() == PropertyDisplay::None) {
        return false;
    }

    auto name = property.get_name().to_string();
    auto value = property.get_value(instance);
    bool changed = input_variant(name.c_str(), value, display);
    if (changed) {
        property.set_value(instance, value);
    }
    return changed;
}

template <typename T, typename Func>
bool input_variant_type(const char *label,
                        rttr::variant &v,
                        bool &changed,
                        Func &&func) {
    auto type = v.get_type();
    auto target_type = rttr::type::get<T>();
    if (type == target_type) {
        auto &value = v.template get_value<T>();
        changed = func(label, value);
        return true;
    } else if (type.is_wrapper() && type.get_wrapped_type() == target_type) {
        // Very strange the rttr returns const & to the wrapped value :(
        // https://github.com/rttrorg/rttr/issues/167
        auto &value = const_cast<T &>(v.template get_wrapped_value<T>());
        changed = func(label, value);
        return true;
    }
    return false;
}

namespace {
bool input_enum_variant(const char *label, rttr::variant &v, bool &changed) {
    auto type = v.get_type();
    if (!type.is_enumeration()) {
        return false;
    }

    auto enum_type = type.get_enumeration();
    auto names = enum_type.get_names();
    auto cur_name = enum_type.value_to_name(v).to_string();
    if (ImGui::BeginCombo(label, cur_name.c_str())) {
        for (auto &n : names) {
            bool is_selected = n == cur_name;
            if (ImGui::Selectable(n.to_string().c_str(), is_selected)) {
                v = enum_type.name_to_value(n);
                changed = true;
            }
            if (is_selected) {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }

    return true;
}

template <typename Int>
bool input_integer_variant_t(const char *label,
                             rttr::variant &v,
                             bool &changed) {
    return input_variant_type<Int>(
        label, v, changed, [&](const char *label, Int &v) {
            int iv = static_cast<int>(v);
            if (ImGui::InputInt(label, &iv)) {
                // Do no change when input a negative value to unsigned int
                if (std::is_unsigned_v<Int> && iv < 0) {
                    return false;
                }
                v = static_cast<Int>(iv);
                return true;
            }
            return false;
        });
}

bool input_integer_variant(const char *label, rttr::variant &v, bool &changed) {
    return input_integer_variant_t<int>(label, v, changed) ||
           input_integer_variant_t<unsigned int>(label, v, changed) ||
           input_integer_variant_t<short>(label, v, changed) ||
           input_integer_variant_t<unsigned short>(label, v, changed) ||
           input_integer_variant_t<size_t>(label, v, changed) ||
           input_integer_variant_t<int8_t>(label, v, changed) ||
           input_integer_variant_t<int16_t>(label, v, changed) ||
           input_integer_variant_t<int32_t>(label, v, changed) ||
           input_integer_variant_t<int64_t>(label, v, changed) ||
           input_integer_variant_t<uint8_t>(label, v, changed) ||
           input_integer_variant_t<uint16_t>(label, v, changed) ||
           input_integer_variant_t<uint32_t>(label, v, changed) ||
           input_integer_variant_t<uint64_t>(label, v, changed);
}
} // namespace

bool input_variant(const char *label,
                   rttr::variant &v,
                   const std::optional<PropertyDisplay> &display) {
    bool changed = false;
    if (input_variant_type<std::string>(label, v, changed, input_text)) {
        return changed;
    }

    if (input_variant_type<bool>(
            label, v, changed, [&](const char *label, bool &value) {
                return ImGui::Checkbox(label, &value);
            })) {
        return changed;
    }

    if (input_variant_type<float>(
            label, v, changed, [&](const char *label, float &v) {
                return ImGui::InputFloat(label, &v);
            })) {
        return changed;
    }

    if (input_integer_variant(label, v, changed)) {
        return changed;
    }

    if (input_variant_type<glm::vec2>(label, v, changed, input_vec2)) {
        return changed;
    }

    if (input_variant_type<glm::vec3>(
            label,
            v,
            changed,
            display == PropertyDisplay::Color ? input_color3 : input_vec3)) {
        return changed;
    }

    if (input_variant_type<glm::vec4>(
            label,
            v,
            changed,
            display == PropertyDisplay::Color ? input_color4 : input_vec4)) {
        return changed;
    }

    if (input_variant_type<glm::mat4>(label, v, changed, input_mat4)) {
        return changed;
    }

    if (input_variant_type<glm::quat>(label, v, changed, input_rotation)) {
        return changed;
    }

    if (input_variant_type<math::XformTRS<float>>(
            label, v, changed, input_xform)) {
        return changed;
    }

    if (input_enum_variant(label, v, changed)) {
        return changed;
    }

    if (input_variant_type<std::shared_ptr<IRes>>(
            label, v, changed, input_res)) {
        return changed;
    }

    if (v.is_sequential_container()) {
        if (ImGui::TreeNode(label)) {
            auto view = v.create_sequential_view();
            auto size = view.get_size();
            std::set<int> to_delete{};
            auto is_dyn = view.is_dynamic();
            for (int i = 0; i < size; i++) {
                ImGui::PushID(i);
                if (is_dyn) {
                    if (ImGui::Button("Delete")) {
                        to_delete.insert(i);
                        changed = true;
                    }
                    ImGui::SameLine();
                }

                auto item = view.get_value(i);
                // Items inherit metadata, otherwise it will be more complex to
                // represent an array of color
                if (input_variant(
                        fmt::format("Item {}", i).c_str(), item, display)) {
                    view.set_value(i, item);
                    changed = true;
                }
                ImGui::PopID();
            }
            for (int i = static_cast<int>(size) - 1; i >= 0; i--) {
                if (to_delete.count(i) > 0) {
                    view.erase(view.begin() + i);
                }
            }
            if (is_dyn) {
                if (ImGui::Button("Add")) {
                    view.set_size(view.get_size() + 1);
                    changed = true;
                }
            }
            ImGui::TreePop();
        }
        return changed;
    }

    if (ImGui::TreeNode(label)) {
        changed = input_instance(rttr::instance(v)) || changed;
        ImGui::TreePop();
        return changed;
    }
    return changed;
}

bool input_res(const char *label, std::shared_ptr<IRes> &v) {
    ImGui::Text("%s", label);
    ImGui::Selectable(v == nullptr ? "<NULL>" : v->path().c_str());
    bool changed = false;
    if (ImGui::BeginDragDropTarget()) {
        auto payload = reinterpret_cast<const char *>(
            ImGui::AcceptDragDropPayload(DRAG_DROP_TARGET_RESOURCES));
        if (payload) {
            v = engine::resources()->load_res(payload);
            changed = true;
        }
        ImGui::EndDragDropTarget();
    }
    return changed;
}
} // namespace ars::gui
