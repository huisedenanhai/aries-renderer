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
    if (ImGui::Button("Reset")) {
        xform = {};
        dirty = true;
    }
    auto t = xform.translation();
    if (input_vec3("T", t)) {
        xform.set_translation(t);
        dirty = true;
    }

    auto r = xform.rotation();
    if (input_rotation("R", r)) {
        xform.set_rotation(r);
        dirty = true;
    }

    auto s = xform.scale();
    if (input_vec3("S", s)) {
        xform.set_scale(s);
        dirty = true;
    }
    return dirty;
}
} // namespace ars::gui
