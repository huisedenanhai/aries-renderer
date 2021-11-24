#pragma once

#include "../Common.h"
#include "../ITexture.h"
#include <ars/runtime/core/Res.h>
#include <filesystem>
#include <memory>

namespace ars::render {
class IContext;

Format get_8_bit_texture_format(uint32_t channels, bool need_srgb);

std::shared_ptr<ITexture> load_texture(IContext *context,
                                       const std::filesystem::path &path);

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

constexpr const char *TEXTURE_RES_TYPE_NAME = "ars::render::ITexture";

std::shared_ptr<ITexture> load_texture(IContext *context, const ResData &data);

} // namespace ars::render