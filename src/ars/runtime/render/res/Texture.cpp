#include "Texture.h"

#include "../IContext.h"
#include "../ITexture.h"
#include <ars/runtime/core/Log.h>
#include <chrono>
#include <sstream>
#include <stb_image.h>

namespace ars::render {
std::unique_ptr<ITexture> load_texture(IContext *context,
                                       const std::filesystem::path &path) {

    using namespace std::chrono;
    auto start = high_resolution_clock::now();
    int width, height, channels;

    auto path_str = path.string();
    unsigned char *data =
        stbi_load(path_str.c_str(), &width, &height, &channels, 4);

    if (!data) {
        std::stringstream ss;
        ss << "Failed to load image " << path;
        panic(ss.str());
    }

    auto texture =
        context->create_texture_2d(Format::R8G8B8A8_SRGB, width, height);

    auto mid = high_resolution_clock::now();

    texture->set_data(
        data, width * height * 4, 0, 0, 0, 0, 0, width, height, 1);
    texture->generate_mipmap();

    stbi_image_free(data);

    auto stop = high_resolution_clock::now();
    auto total_duration = duration_cast<milliseconds>(stop - start);
    auto decode_duration = duration_cast<milliseconds>(mid - start);
    auto upload_duration = duration_cast<milliseconds>(stop - mid);

    {
        std::stringstream ss;
        ss << "Load texture " << path << " " << width << "x" << height
           << " takes " << total_duration.count() << "ms, decode image takes "
           << decode_duration.count() << "ms, upload takes "
           << upload_duration.count() << "ms" << std::endl;
        log_info(ss.str());
    }

    return texture;
}
} // namespace ars::render