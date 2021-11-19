#include "FileBrowser.h"
#include <ars/runtime/core/gui/ImGui.h>
#include <imgui/imgui.h>

namespace ars::editor {
namespace {
void tree_view_directory(FileBrowserState &state,
                         const std::filesystem::path &root,
                         std::filesystem::path &current_selected) {
    for (auto const &entry : std::filesystem::directory_iterator{root}) {
        const auto &path = entry.path();
        auto filename = path.filename().string();
        if (!filename.empty() && filename[0] == '.') {
            continue;
        }
        if (gui::begin_selectable_tree_node(path.string().c_str(),
                                            filename.c_str(),
                                            !entry.is_directory(),
                                            path,
                                            current_selected)) {
            if (entry.is_directory()) {
                tree_view_directory(state, path, current_selected);
            } else {
                if (ImGui::IsItemHovered() &&
                    ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
                    state.file_open_callback(path);
                }
            }
            ImGui::TreePop();
        }
    }
}
} // namespace

void file_browser(FileBrowserState &state,
                  const std::filesystem::path &root,
                  std::filesystem::path &current_selected) {
    if (!std::filesystem::is_directory(root)) {
        return;
    }

    tree_view_directory(state, root, current_selected);
}
} // namespace ars::editor