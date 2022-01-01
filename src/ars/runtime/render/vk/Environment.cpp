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

std::shared_ptr<ITexture> Environment::irradiance_cube_map() {
    if (_irradiance_cube_map == nullptr) {
        return _context->default_texture(DefaultTexture::WhiteCubeMap);
    }
    return _irradiance_cube_map;
}

void Environment::alloc_irradiance_cube_map(uint32_t resolution) {
    IContext *ctx_base = _context;
    // Max mip level 5.
    // Using full mip map count degenerates diffuse irradiance to ambient cube.
    _irradiance_cube_map =
        ctx_base->create_texture(TextureInfo::create_cube_map(
            Format::B10G11R11_UFLOAT_PACK32, resolution, 5));
}

Environment::Environment(Context *context) : _context(context) {
    _prefilter_env_pipeline =
        ComputePipeline::create(context, "PrefilterCubeMap.comp");
    _capture_cube_map_pipeline =
        ComputePipeline::create(context, "CaptureHDRToCubeMap.comp");
}

void Environment::update_irradiance_cube_map() {
    if (_irradiance_cube_map == nullptr) {
        ARS_LOG_ERROR("Failed to update environment cube map: Cube map not "
                      "initialized.");
        return;
    }

    auto irradiance_map = upcast(_irradiance_cube_map.get());

    // Alloc a tmp cube map for holding mip-mapped radiance
    IContext *ctx_base = _context;
    auto tmp_env_map = ctx_base->create_texture(TextureInfo::create_cube_map(
        Format::B10G11R11_UFLOAT_PACK32, _irradiance_cube_map->width()));
    auto env_map = upcast(tmp_env_map.get());

    _context->queue()->submit_once([&](CommandBuffer *cmd) {
        irradiance_map->transfer_layout(cmd,
                                        VK_IMAGE_LAYOUT_GENERAL,
                                        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                                        0,
                                        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                                        VK_ACCESS_SHADER_WRITE_BIT |
                                            VK_ACCESS_SHADER_READ_BIT);
        env_map->transfer_layout(cmd,
                                 VK_IMAGE_LAYOUT_GENERAL,
                                 VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                                 0,
                                 VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                                 VK_ACCESS_SHADER_WRITE_BIT |
                                     VK_ACCESS_SHADER_READ_BIT);

        capture_env_cube_map(cmd, env_map);

        env_map->generate_mipmap(cmd,
                                 VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                                 VK_ACCESS_SHADER_WRITE_BIT,
                                 VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                                 VK_ACCESS_SHADER_READ_BIT);

        prefilter_irradiance(cmd, env_map);
    });
}

Handle<Texture> Environment::irradiance_cube_map_vk() {
    return upcast(irradiance_cube_map().get());
}

Handle<Texture> Environment::hdr_texture_vk() {
    return upcast(hdr_texture().get());
}

void Environment::set_irradiance_cube_map(
    const std::shared_ptr<ITexture> &cube_map) {
    if (cube_map->type() != TextureType::CubeMap) {
        ARS_LOG_ERROR("Failed to set cube map of Environment: Not a cube map");
        return;
    }
    _irradiance_cube_map = cube_map;
}

void Environment::capture_env_cube_map(CommandBuffer *cmd,
                                       const Handle<Texture> &env_map) {
    _capture_cube_map_pipeline->bind(cmd);

    DescriptorEncoder desc{};
    desc.set_texture(0, 0, hdr_texture_vk().get());
    desc.set_texture(0, 1, env_map.get());
    auto size = static_cast<int32_t>(env_map->info().extent.width);
    desc.set_buffer_data(0, 2, size);
    desc.commit(cmd, _capture_cube_map_pipeline.get());

    _capture_cube_map_pipeline->local_size().dispatch(cmd, size, size, 6);
}

void Environment::prefilter_irradiance(CommandBuffer *cmd,
                                       const Handle<Texture> &env_map) {
    _prefilter_env_pipeline->bind(cmd);
    {
        DescriptorEncoder desc{};
        desc.set_texture(0, 0, env_map.get());
        desc.commit(cmd, _prefilter_env_pipeline.get());
    }

    uint32_t mip_count = _irradiance_cube_map->mip_levels();
    uint32_t size = _irradiance_cube_map->width();

    for (int i = 0; i < mip_count; i++) {
        struct Param {
            int32_t size;
            float roughness;
            int32_t mip_level;
            int32_t radiance_base_resolution;
        };

        Param param{};
        param.size = static_cast<int32_t>(size);
        param.roughness = static_cast<float>(i) /
                          static_cast<float>(std::max(1u, mip_count - 1));
        param.mip_level = i;
        param.radiance_base_resolution =
            static_cast<int32_t>(env_map->info().extent.width);

        DescriptorEncoder desc{};
        desc.set_buffer_data(1, 0, param);
        desc.set_texture(1, 1, irradiance_cube_map_vk().get(), i);
        desc.commit(cmd, _prefilter_env_pipeline.get());

        _prefilter_env_pipeline->local_size().dispatch(cmd, size, size, 6);

        size = (size + 1) / 2;
    }
}

Environment::~Environment() = default;
} // namespace ars::render::vk
