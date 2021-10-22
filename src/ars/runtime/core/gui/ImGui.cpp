#include "ImGui.h"

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
} // namespace ars::gui
