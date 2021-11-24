#include <ars/runtime/core/Log.h>
#include <ars/runtime/core/Res.h>
#include <ars/runtime/render/res/Texture.h>
#include <iostream>
#include <stb_image.h>

using namespace ars;

std::optional<ResData> import_texture(const std::filesystem::path &path) {
    int width, height, channels;
    auto path_str = path.string();
    // Only loads image as R8G8B8A8_SRGB for simplicity. Other formats may not
    // be supported on some devices.
    unsigned char *pixel_data =
        stbi_load(path_str.c_str(), &width, &height, &channels, 4);
    channels = 4;

    if (!pixel_data) {
        ARS_LOG_CRITICAL("Failed to load image {}", path.string());
        return std::nullopt;
    }

    render::TextureResMeta meta{};
    auto &info = meta.info;
    info.format = render::get_8_bit_texture_format(channels, true);
    info.width = width;
    info.height = height;

    DataSlice pixels{};
    pixels.offset = 0;
    pixels.size = width * height * channels;

    meta.layers.resize(1);
    meta.layers[0].mipmaps.push_back(pixels);

    ResData res{};
    res.ty = render::TEXTURE_RES_TYPE_NAME;
    res.meta = meta;
    res.data.resize(pixels.size);
    std::memcpy(res.data.data(), pixel_data, pixels.size);

    return res;
}

int main() {
    auto res = import_texture("test.jpg");
    if (res.has_value()) {
        res->save(preferred_res_path("test.jpg"));
    }
}