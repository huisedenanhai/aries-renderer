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
    uint32_t mip_levels = MAX_MIP_LEVELS;
    uint32_t array_layers = 1;
};

uint32_t calculate_mip_levels(uint32_t width, uint32_t height, uint32_t depth);

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

    virtual void set_data(void *data,
                          size_t size,
                          uint32_t mip_level,
                          uint32_t layer,
                          uint32_t x_offset,
                          uint32_t y_offset,
                          uint32_t z_offset,
                          uint32_t x_size,
                          uint32_t y_size,
                          uint32_t z_size) = 0;

    virtual void generate_mipmap() = 0;

  protected:
    TextureInfo _info{};
};
} // namespace ars::render
