#include "Environment.h"
#include "Context.h"

namespace ars::render::vk {
std::shared_ptr<ITexture> Environment::hdr_texture() {
    if (_hdr_texture == nullptr) {
        return _context->default_texture(DefaultTexture::White);
    }
    return _hdr_texture;
}

void Environment::set_hdr_texture(const std::shared_ptr<ITexture> &hdr) {
    _hdr_texture = hdr;
}

glm::vec3 Environment::radiance() {
    return _radiance;
}

void Environment::set_radiance(const glm::vec3 &radiance) {
    _radiance = radiance;
}

std::shared_ptr<ITexture> Environment::cube_map() {
    if (_cube_map == nullptr) {
        return _context->default_texture(DefaultTexture::WhiteCubeMap);
    }
    return _cube_map;
}

void Environment::set_cube_map_resolution(uint32_t resolution) {
    // Already same sized
    if (_cube_map != nullptr && _cube_map->width() == resolution) {
        return;
    }
    IContext *ctx_base = _context;
    _cube_map = ctx_base->create_texture(TextureInfo::create_cube_map(
        Format::B10G11R11_UFLOAT_PACK32, resolution));
}

Environment::Environment(Context *context) : _context(context) {}

void Environment::update_cache() {
    if (_cube_map == nullptr) {
        IContext *ctx_base = _context;
        _cube_map = ctx_base->create_texture(
            TextureInfo::create_cube_map(Format::B10G11R11_UFLOAT_PACK32, 1));
    }

    auto cube_map = upcast(_cube_map.get());
    _context->queue()->submit_once([&](CommandBuffer *cmd) {
        cube_map->transfer_layout(cmd,
                                  VK_IMAGE_LAYOUT_GENERAL,
                                  VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                                  0,
                                  VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                                  VK_ACCESS_SHADER_WRITE_BIT |
                                      VK_ACCESS_SHADER_READ_BIT);
    });
}

Handle<Texture> Environment::cube_map_vk() {
    return upcast(cube_map().get());
}

Handle<Texture> Environment::hdr_texture_vk() {
    return upcast(hdr_texture().get());
}
} // namespace ars::render::vk
