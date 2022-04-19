#pragma once

#include <filesystem>
#include <functional>

namespace ars::editor {
struct FileBrowserState {
    std::string filter_pattern{};
    std::function<void(const std::filesystem::path &)> on_file_open =
        [](auto &&) {};
};

void file_browser(FileBrowserState &state,
                  const std::filesystem::path &root,
                  std::filesystem::path &current_selected);
} // namespace ars::editor