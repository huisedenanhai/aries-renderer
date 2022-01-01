#include "Environment.h"
#include "Context.h"
#include "Pipeline.h"
#include "ars/runtime/core/Log.h"

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

void Environment::alloc_cube_map(uint32_t resolution) {
    IContext *ctx_base = _context;
    // Max mip level 5.
    // Using full mip map count degenerates diffuse irradiance to ambient cube.
    _cube_map = ctx_base->create_texture(TextureInfo::create_cube_map(
        Format::B10G11R11_UFLOAT_PACK32, resolution, 5));
}

Environment::Environment(Context *context) : _context(context) {
    _prefilter_env_pipeline =
        ComputePipeline::create(context, "PrefilterCubeMap.comp");
}

void Environment::update_cube_map() {
    if (_cube_map == nullptr) {
        ARS_LOG_ERROR("Failed to update environment cube map: Cube map not "
                      "initialized.");
        return;
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

        _prefilter_env_pipeline->bind(cmd);
        {
            DescriptorEncoder desc{};
            desc.set_texture(0, 0, hdr_texture_vk().get());
            desc.commit(cmd, _prefilter_env_pipeline.get());
        }

        uint32_t mip_count = _cube_map->mip_levels();
        uint32_t size = _cube_map->width();

        for (int i = 0; i < mip_count; i++) {
            struct Param {
                int32_t size;
                float roughness;
                int32_t mip_level;
                ARS_PADDING_FIELD(int32_t);
            };

            Param param{};
            param.size = static_cast<int32_t>(size);
            param.roughness = static_cast<float>(i) /
                              static_cast<float>(std::max(1u, mip_count - 1));
            param.mip_level = i;

            DescriptorEncoder desc{};
            desc.set_buffer_data(1, 0, param);
            desc.set_texture(1, 1, cube_map_vk().get(), i);
            desc.commit(cmd, _prefilter_env_pipeline.get());

            _prefilter_env_pipeline->local_size().dispatch(cmd, size, size, 6);

            size = (size + 1) / 2;
        }
    });
}

Handle<Texture> Environment::cube_map_vk() {
    return upcast(cube_map().get());
}

Handle<Texture> Environment::hdr_texture_vk() {
    return upcast(hdr_texture().get());
}

void Environment::set_cube_map(const std::shared_ptr<ITexture> &cube_map) {
    if (cube_map->type() != TextureType::CubeMap) {
        ARS_LOG_ERROR("Failed to set cube map of Environment: Not a cube map");
        return;
    }
    _cube_map = cube_map;
}

Environment::~Environment() = default;
} // namespace ars::render::vk
