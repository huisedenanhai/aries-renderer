#pragma once

#include "../math/Transform.h"
#include <glm/glm.hpp>
#include <imgui/imgui.h>
#include <rttr/type>
#include <string>

namespace ars::gui {
bool input_text(const char *label, std::string &str);
bool input_vec2(const char *label, glm::vec2 &v);
bool input_vec3(const char *label, glm::vec3 &v);
bool input_vec4(const char *label, glm::vec4 &v);
bool input_color3(const char *label, glm::vec3 &v);
bool input_color4(const char *label, glm::vec4 &v);
bool input_rotation(const char *label, glm::quat &q);
bool input_xform(const char *label, math::XformTRS<float> &xform);
bool input_instance(rttr::instance instance);

enum class PropertyDisplay {
    // Works for vec3, vec4
    Color
};

enum class PropertyAttribute {
    // Should be PropertyDisplay
    Display
};

bool input_property(rttr::instance instance, rttr::property property);
} // namespace ars::gui