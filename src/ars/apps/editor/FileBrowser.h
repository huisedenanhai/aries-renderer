#pragma once

#include <filesystem>

namespace ars::editor {
struct FileBrowserState {};

void file_browser(FileBrowserState &state,
                  const std::filesystem::path &root,
                  std::filesystem::path &current_selected);
} // namespace ars::editor