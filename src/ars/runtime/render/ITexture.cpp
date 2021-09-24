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

FilterMode ITexture::min_filter() const {
    return _info.min_filter;
}

FilterMode ITexture::mag_filter() const {
    return _info.mag_filter;
}

MipmapMode ITexture::mipmap_mode() const {
    return _info.mipmap_mode;
}

WrapMode ITexture::wrap_u() const {
    return _info.wrap_u;
}

WrapMode ITexture::wrap_v() const {
    return _info.wrap_v;
}

WrapMode ITexture::wrap_w() const {
    return _info.wrap_w;
}

uint32_t calculate_mip_levels(uint32_t width, uint32_t height, uint32_t depth) {
    return static_cast<uint32_t>(
               std::floor(std::log2(std::max({width, height, depth})))) +
           1;
}
} // namespace ars::render