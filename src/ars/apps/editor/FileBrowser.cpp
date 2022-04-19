#include "FileBrowser.h"
#include <ars/runtime/engine/gui/ImGui.h>
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
        if (!entry.is_directory() &&
            filename.find(state.filter_pattern) == std::string::npos) {
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
                    state.on_file_open(path);
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
    gui::input_text("Filter", state.filter_pattern);
    ImGui::Separator();
    if (ImGui::BeginChild("File Tree View")) {
        if (std::filesystem::is_directory(root)) {
            tree_view_directory(state, root, current_selected);
        }
        ImGui::EndChild();
    }
}
} // namespace ars::editor