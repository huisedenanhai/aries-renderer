#include "ITexture.h"
#include <algorithm>
#include <cmath>

namespace ars::render {
ITexture::ITexture(const TextureInfo &info) : _info(info) {}

TextureType ITexture::type() const {
    return _info.type;
}

Format ITexture::format() const {
    return _info.format;
}

uint32_t ITexture::width() const {
    return _info.width;
}

uint32_t ITexture::height() const {
    return _info.height;
}

uint32_t ITexture::depth() const {
    return _info.depth;
}

uint32_t ITexture::mip_levels() const {
    return _info.mip_levels;
}

uint32_t ITexture::array_layers() const {
    return _info.array_layers;
}

uint32_t calculate_mip_levels(uint32_t width, uint32_t height, uint32_t depth) {
    return static_cast<uint32_t>(
               std::floor(std::log2(std::max({width, height, depth})))) +
           1;
}
} // namespace ars::render