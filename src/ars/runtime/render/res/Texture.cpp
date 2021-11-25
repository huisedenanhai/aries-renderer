#include "Texture.h"

#include "../IContext.h"
#include <ars/runtime/core/Log.h>
#include <chrono>
#include <stb_image.h>

namespace ars::render {
Format get_8_bit_texture_format(uint32_t channels, bool need_srgb) {
    Format formats[8] = {
        Format::R8_SRGB,
        Format::R8_UNORM,
        Format::R8G8_SRGB,
        Format::R8G8_UNORM,
        Format::R8G8B8_SRGB,
        Format::R8G8B8_UNORM,
        Format::R8G8B8A8_SRGB,
        Format::R8G8B8A8_UNORM,
    };

    return formats[(channels - 1) * 2 + (need_srgb ? 0 : 1)];
}

std::shared_ptr<ITexture> load_texture(IContext *context,
                                       const std::filesystem::path &path) {

    using namespace std::chrono;
    auto start = high_resolution_clock::now();
    int width, height, channels;

    auto path_str = path.string();
    // Only loads image as R8G8B8A8_SRGB for simplicity. Other formats may not
    // be supported on some devices.
    unsigned char *data =
        stbi_load(path_str.c_str(), &width, &height, &channels, 4);
    channels = 4;

    if (!data) {
        ARS_LOG_CRITICAL("Failed to load image {}", path.string());
        return nullptr;
    }

    auto texture = context->create_texture_2d(
        get_8_bit_texture_format(channels, true), width, height);

    auto mid = high_resolution_clock::now();

    texture->set_data(
        data, width * height * channels, 0, 0, 0, 0, 0, width, height, 1);
    texture->generate_mipmap();

    stbi_image_free(data);

    auto stop = high_resolution_clock::now();
    auto total_duration = duration_cast<milliseconds>(stop - start);
    auto decode_duration = duration_cast<milliseconds>(mid - start);
    auto upload_duration = duration_cast<milliseconds>(stop - mid);

    ARS_LOG_INFO("Load texture {} {}x{} takes {}ms, decode image takes "
                 "{}ms, upload takes {}ms",
                 path.string(),
                 width,
                 height,
                 total_duration.count(),
                 decode_duration.count(),
                 upload_duration.count());

    return texture;
}

std::shared_ptr<ITexture> load_texture(IContext *context, const ResData &data) {
    if (data.ty != RES_TYPE_NAME_TEXTURE) {
        ARS_LOG_ERROR("Failed to load texture: invalid data type");
        return nullptr;
    }
    TextureResMeta meta{};
    data.meta.get_to(meta);
    auto tex = context->create_texture(meta.info);
    for (int l = 0; l < meta.layers.size(); l++) {
        auto &layer = meta.layers[l];

        auto mip_width = static_cast<int32_t>(meta.info.width);
        auto mip_height = static_cast<int32_t>(meta.info.height);
        auto mip_depth = static_cast<int32_t>(meta.info.depth);

        for (int m = 0; m < layer.mipmaps.size(); m++) {
            auto calc_next_mip_size = [](int32_t size) {
                return size > 1 ? size / 2 : 1;
            };
            auto &mip = layer.mipmaps[m];
            tex->set_data((void *)(&data.data[mip.offset]),
                          mip.size,
                          m,
                          l,
                          0,
                          0,
                          0,
                          mip_width,
                          mip_height,
                          mip_depth);
            mip_width = calc_next_mip_size(mip_width);
            mip_height = calc_next_mip_size(mip_height);
            mip_depth = calc_next_mip_size(mip_depth);
        }
    }

    if (meta.regenerate_mipmap) {
        tex->generate_mipmap();
    }

    return tex;
}
} // namespace ars::render