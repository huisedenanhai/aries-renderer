#pragma once

#include "../Common.h"
#include "../ITexture.h"
#include <ars/runtime/core/ResData.h>
#include <filesystem>
#include <memory>

namespace ars::render {
class IContext;

Format get_8_bit_texture_format(uint32_t channels, bool need_srgb);

std::shared_ptr<ITexture> load_texture(IContext *context,
                                       const std::filesystem::path &path);

// DO NOT put this in ITexture.h, it slows down compilation significantly
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(TextureInfo,
                                   format,
                                   width,
                                   height,
                                   depth,
                                   mip_levels,
                                   array_layers,
                                   min_filter,
                                   mag_filter,
                                   mipmap_mode,
                                   wrap_u,
                                   wrap_v,
                                   wrap_w)

struct TextureResMeta {
    TextureInfo info{};

    struct Layer {
        std::vector<DataSlice> mipmaps{};
        NLOHMANN_DEFINE_TYPE_INTRUSIVE(Layer, mipmaps)
    };

    std::vector<Layer> layers{};
    bool regenerate_mipmap = true;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(TextureResMeta, info, layers)
};

constexpr const char *RES_TYPE_NAME_TEXTURE = "ars::render::ITexture";

std::shared_ptr<ITexture> load_texture(IContext *context, const ResData &data);

} // namespace ars::render