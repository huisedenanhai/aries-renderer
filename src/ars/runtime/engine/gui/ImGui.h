#pragma once

#include <ars/runtime/core/Reflect.h>
#include <ars/runtime/core/Res.h>
#include <ars/runtime/core/math/Transform.h>
#include <glm/glm.hpp>
#include <imgui/imgui.h>
#include <optional>
#include <rttr/type>
#include <string>

namespace ars::gui {
// Payload: const char*
constexpr const char *DRAG_DROP_TARGET_RESOURCES = "ARS_DRAG_DROP_RESOURCES";

bool input_text(const char *label, std::string &str);
bool input_vec2(const char *label, glm::vec2 &v);
bool input_vec3(const char *label, glm::vec3 &v);
bool input_vec4(const char *label, glm::vec4 &v);
bool input_color3(const char *label, glm::vec3 &v);
bool input_color4(const char *label, glm::vec4 &v);
bool input_rotation(const char *label, glm::quat &q);
bool input_xform(const char *label, math::XformTRS<float> &xform);
bool input_res(const char *label, std::shared_ptr<IRes> &v);
bool input_instance(const rttr::instance &instance);

bool input_variant(
    const char *label,
    rttr::variant &v,
    const std::optional<PropertyDisplay> &display = std::nullopt);

bool input_property(const rttr::instance &instance, rttr::property property);

template <typename T, typename Id>
bool begin_selectable_tree_node(
    Id id, const char *name, bool is_leaf, const T &v, T &current_selected) {
    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnDoubleClick;
    flags |= v == current_selected ? ImGuiTreeNodeFlags_Selected : 0;
    flags |= is_leaf ? ImGuiTreeNodeFlags_Leaf : 0;

    bool opened = ImGui::TreeNodeEx(id, flags, "%s", name);

    if (ImGui::IsItemClicked(ImGuiMouseButton_Left) ||
        ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
        current_selected = v;
    }

    return opened;
}

} // namespace ars::gui