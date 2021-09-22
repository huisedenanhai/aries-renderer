#pragma once

#include <filesystem>
#include <memory>

namespace ars::render {
class ITexture;
class IContext;

std::unique_ptr<ITexture> load_texture(IContext *context,
                                       const std::filesystem::path &path);

} // namespace ars::render