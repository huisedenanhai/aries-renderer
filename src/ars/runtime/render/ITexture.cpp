#include "ITexture.h"
#include <algorithm>
#include <cmath>
#include <rttr/registration>

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

void ITexture::register_type() {
    rttr::registration::class_<ITexture>("ars::render::ITexture");
}

uint32_t calculate_mip_levels(uint32_t width, uint32_t height, uint32_t depth) {
    return static_cast<uint32_t>(
               std::floor(std::log2(std::max({width, height, depth})))) +
           1;
}

uint32_t calculate_next_mip_size(uint32_t size) {
    return size > 1 ? size / 2 : 1;
}

TextureInfo TextureInfo::create(const std::string &name,
                                TextureType type,
                                Format format,
                                uint32_t width,
                                uint32_t height,
                                uint32_t mip_levels) {
    TextureInfo info{};
    info.name = name;
    info.format = format;
    info.type = type;
    info.width = width;
    info.height = height;
    info.depth = 1;
    info.mip_levels = mip_levels;
    info.array_layers = 1;
    return info;
}

TextureInfo TextureInfo::create_2d(const std::string &name,
                                   Format format,
                                   uint32_t width,
                                   uint32_t height,
                                   uint32_t mip_levels) {
    return create(
        name, TextureType::Texture2D, format, width, height, mip_levels);
}

TextureInfo TextureInfo::create_cube_map(const std::string &name,
                                         Format format,
                                         uint32_t size,
                                         uint32_t mip_levels) {
    return create(name, TextureType::CubeMap, format, size, size, mip_levels);
}
} // namespace ars::render