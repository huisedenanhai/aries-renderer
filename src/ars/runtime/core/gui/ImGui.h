#pragma once

#include <imgui/imgui.h>
#include <string>

namespace ars::gui {
bool input_text(const char *label, std::string &str);
}