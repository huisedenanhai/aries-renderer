#pragma once

#include "Common.h"
#include <memory>

namespace ars::render {
enum class TextureType { Texture2D };

struct TextureInfo {
    TextureType type = TextureType::Texture2D;
    Format format = Format::R8G8B8A8Srgb;
    uint32_t width = 1;
    uint32_t height = 1;
    uint32_t depth = 1;
    uint32_t mip_levels = 1;
    uint32_t array_layers = 1;
};

class ITexture {
  public:
    explicit ITexture(const TextureInfo &info);

    virtual ~ITexture() = default;

    TextureType type() const;
    Format format() const;
    uint32_t width() const;
    uint32_t height() const;
    uint32_t depth() const;
    uint32_t mip_levels() const;
    uint32_t array_layers() const;

  protected:
    TextureInfo _info{};
};
} // namespace ars::render
