#pragma once

#include <ars/runtime/engine/Entity.h>

namespace ars::editor {
struct SaveAsModalState {
    bool open = false;
    engine::Entity *entity = nullptr;
    std::string input_buffer{};
    std::filesystem::path extension{};
    const char *window_id = nullptr;
};

void open_save_as_modal(SaveAsModalState &state,
                        const char *id,
                        const std::filesystem::path &extension,
                        engine::Entity *entity);

void save_as_modal(SaveAsModalState &state,
                   std::optional<std::filesystem::path> &save_dir);
} // namespace ars::editor