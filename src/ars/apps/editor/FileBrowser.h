#pragma once

#include <filesystem>

namespace ars::editor {
struct FileBrowserState {
    std::function<void(const std::filesystem::path &)> on_file_open =
        [](auto &&) {};
};

void file_browser(FileBrowserState &state,
                  const std::filesystem::path &root,
                  std::filesystem::path &current_selected);
} // namespace ars::editor