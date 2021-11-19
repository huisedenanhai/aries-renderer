#include "SaveAsModal.h"
#include <ars/runtime/core/gui/ImGui.h>
#include <imgui/imgui.h>

namespace ars::editor {
void open_save_as_modal(SaveAsModalState &state,
                        const char *id,
                        const std::filesystem::path &extension,
                        engine::Entity *entity) {
    state.window_id = id;
    state.input_buffer = "untitled";
    state.entity = entity;
    state.extension = extension;
    state.open = true;
    ImGui::OpenPopup(id);
}

void save_as_modal(SaveAsModalState &state,
                   std::optional<std::filesystem::path> &save_dir) {
    if (!state.open) {
        return;
    } else {
        ImGui::OpenPopup(state.window_id);
    }
    if (ImGui::BeginPopupModal(state.window_id)) {
        if (state.entity == nullptr) {
            ImGui::Text("Please select an entity for save");
            if (ImGui::Button("OK")) {
                ImGui::CloseCurrentPopup();
            }
        } else {
            gui::input_text("Path", state.input_buffer);
            bool close = false;
            if (ImGui::Button("Cancel")) {
                close = true;
            }
            ImGui::SameLine();
            if (ImGui::Button("Save")) {
                close = true;
                save_dir = state.input_buffer;
                if (save_dir->extension() != state.extension) {
                    save_dir->replace_extension(state.extension);
                }
                state.entity->save(save_dir.value());
            }
            if (close) {
                state.open = false;
                ImGui::CloseCurrentPopup();
            }
        }

        ImGui::EndPopup();
    }
}
} // namespace ars::editor