#pragma once

#include "Common.h"
#include "IWindow.h"
#include <ars/runtime/core/Reflect.h>
#include <ars/runtime/core/Res.h>
#include <memory>

namespace ars::render {
enum class TextureType { Texture2D, CubeMap };

enum class FilterMode { Linear, Nearest };

enum class MipmapMode { Linear, Nearest };

enum class WrapMode { Repeat, ClampToEdge, MirroredRepeat };

struct TextureInfo {
    std::string name{};
    TextureType type = TextureType::Texture2D;
    Format format = Format::R8G8B8A8_SRGB;
    uint32_t width = 1;
    uint32_t height = 1;
    uint32_t depth = 1;
    uint32_t mip_levels = MAX_MIP_LEVELS;
    uint32_t array_layers = 1;

    FilterMode min_filter = FilterMode::Linear;
    FilterMode mag_filter = FilterMode::Linear;
    MipmapMode mipmap_mode = MipmapMode::Linear;
    WrapMode wrap_u = WrapMode::Repeat;
    WrapMode wrap_v = WrapMode::Repeat;
    WrapMode wrap_w = WrapMode::Repeat;

    static TextureInfo create(const std::string &name,
                              TextureType type,
                              Format format,
                              uint32_t width,
                              uint32_t height,
                              uint32_t mip_levels = MAX_MIP_LEVELS);
    static TextureInfo create_2d(const std::string &name,
                                 Format format,
                                 uint32_t width,
                                 uint32_t height,
                                 uint32_t mip_levels = MAX_MIP_LEVELS);
    // Width and height for cube map must be equal
    static TextureInfo create_cube_map(const std::string &name,
                                       Format format,
                                       uint32_t size,
                                       uint32_t mip_levels = MAX_MIP_LEVELS);
};

uint32_t calculate_mip_levels(uint32_t width, uint32_t height, uint32_t depth);
uint32_t calculate_next_mip_size(uint32_t size);

class ITextureHandle {
  public:
    virtual ~ITextureHandle() = default;
};

class ITexture : public IRes {
    RTTR_DERIVE(IRes);

  public:
    explicit ITexture(const TextureInfo &info);

    [[nodiscard]] TextureType type() const;
    [[nodiscard]] Format format() const;
    [[nodiscard]] uint32_t width() const;
    [[nodiscard]] uint32_t height() const;
    [[nodiscard]] uint32_t depth() const;
    [[nodiscard]] uint32_t mip_levels() const;
    [[nodiscard]] uint32_t array_layers() const;
    [[nodiscard]] FilterMode min_filter() const;
    [[nodiscard]] FilterMode mag_filter() const;
    [[nodiscard]] MipmapMode mipmap_mode() const;
    [[nodiscard]] WrapMode wrap_u() const;
    [[nodiscard]] WrapMode wrap_v() const;
    [[nodiscard]] WrapMode wrap_w() const;

    // ITexture might only be a wrapper of the actual graphics resources.
    // Resources are delayed destructed. Handles will be valid until the end of
    // the frame.
    [[nodiscard]] virtual ITextureHandle *handle() = 0;

    virtual void set_data(void *data,
                          size_t size,
                          uint32_t mip_level,
                          uint32_t layer,
                          int32_t x_offset,
                          int32_t y_offset,
                          int32_t z_offset,
                          uint32_t x_size,
                          uint32_t y_size,
                          uint32_t z_size) = 0;

    virtual void generate_mipmap() = 0;

    static void register_type();

  protected:
    TextureInfo _info{};
};

} // namespace ars::render
