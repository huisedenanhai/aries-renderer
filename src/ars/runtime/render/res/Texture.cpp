#include "Texture.h"

#include "../IContext.h"
#include "../ITexture.h"
#include <ars/runtime/core/Log.h>
#include <chrono>
#include <sstream>
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
} // namespace ars::render