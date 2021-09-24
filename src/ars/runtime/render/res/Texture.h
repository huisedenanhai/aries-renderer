#pragma once

#include "../Common.h"
#include <filesystem>
#include <memory>

namespace ars::render {
class ITexture;
class IContext;

Format get_8_bit_texture_format(uint32_t channels, bool need_srgb);

std::shared_ptr<ITexture> load_texture(IContext *context,
                                       const std::filesystem::path &path);

} // namespace ars::render