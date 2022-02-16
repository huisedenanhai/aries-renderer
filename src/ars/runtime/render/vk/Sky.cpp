#include "Sky.h"
#include "Context.h"
#include "Pipeline.h"
#include "RenderGraph.h"

namespace ars::render::vk {
Handle<Texture> SkyData::irradiance_cube_map() {
    if (_irradiance_cube_map == nullptr) {
        return upcast(
            _context->default_texture(DefaultTexture::WhiteCubeMap).get());
    }
    return _irradiance_cube_map;
}

PanoramaSky::PanoramaSky(Context *context)
    : _context(context), SkyBase(context) {}

std::shared_ptr<ITexture> PanoramaSky::panorama() {
    return _panorama;
}

void PanoramaSky::set_panorama(std::shared_ptr<ITexture> hdr) {
    data()->set_panorama(upcast(hdr.get()));
    _panorama = hdr;
}

uint32_t PanoramaSky::irradiance_cube_map_size() {
    return data()->irradiance_cube_map_size();
}

void PanoramaSky::set_irradiance_cube_map_size(uint32_t resolution) {
    data()->set_irradiance_cube_map_size(resolution);
}

void SkyData::capture_env_cube_map(CommandBuffer *cmd,
                                   const Handle<Texture> &panorama_tex,
                                   const Handle<Texture> &env_map) {
    auto size = static_cast<int32_t>(env_map->info().extent.width);
    _capture_cube_map_pipeline->bind(cmd);

    DescriptorEncoder desc{};
    desc.set_texture(0, 0, panorama_tex.get());
    desc.set_texture(0, 1, env_map.get());
    desc.commit(cmd, _capture_cube_map_pipeline.get());

    _capture_cube_map_pipeline->local_size().dispatch(cmd, size, size, 6);
}

void SkyData::prefilter_irradiance(CommandBuffer *cmd,
                                   const Handle<Texture> &env_map,
                                   const Handle<Texture> &irradiance_map) {
    assert(irradiance_map != nullptr);

    _prefilter_env_pipeline->bind(cmd);
    {
        DescriptorEncoder desc{};
        desc.set_texture(0, 0, env_map.get());
        desc.commit(cmd, _prefilter_env_pipeline.get());
    }

    uint32_t mip_count = irradiance_map->info().mip_levels;
    uint32_t size = irradiance_map->info().extent.width;

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
        desc.set_texture(1, 1, irradiance_map.get(), i);
        desc.commit(cmd, _prefilter_env_pipeline.get());

        _prefilter_env_pipeline->local_size().dispatch(cmd, size, size, 6);

        size = calculate_next_mip_size(size);
    }
}

Handle<Texture> SkyData::panorama() {
    if (_panorama_texture == nullptr) {
        return upcast(_context->default_texture(DefaultTexture::White).get());
    }
    return _panorama_texture;
}

void SkyData::set_panorama(const Handle<Texture> &hdr) {
    if (_panorama_texture == hdr) {
        return;
    }

    // Reset to default
    if (hdr == nullptr) {
        _panorama_texture = nullptr;
        _irradiance_cube_map = nullptr;
        _tmp_env_map = nullptr;
        _panorama_dirty = false;
        return;
    }

    _panorama_texture = hdr;
    _panorama_dirty = true;
}

uint32_t SkyData::irradiance_cube_map_size() const {
    return _irradiance_cube_map_size;
}

void SkyData::set_irradiance_cube_map_size(uint32_t resolution) {
    _irradiance_cube_map_size = resolution;
}

void SkyData::ensure_irradiance_cube_map() {
    if (_irradiance_cube_map != nullptr &&
        _irradiance_cube_map->info().extent.width ==
            _irradiance_cube_map_size) {
        return;
    }

    // Max mip level 5.
    // Using full mip map count degenerates diffuse irradiance to ambient
    // cube.
    _irradiance_cube_map =
        _context->create_texture(translate(TextureInfo::create_cube_map(
            Format::B10G11R11_UFLOAT_PACK32, _irradiance_cube_map_size, 5)));
    _tmp_env_map =
        _context->create_texture(translate(TextureInfo::create_cube_map(
            Format::B10G11R11_UFLOAT_PACK32, _irradiance_cube_map_size)));
}

void SkyData::update_cache(RenderGraph &rg) {
    if (!cache_dirty()) {
        return;
    }

    ensure_irradiance_cube_map();
    _panorama_dirty = false;

    assert(_panorama_texture != nullptr);
    rg.add_pass(
        [&](RenderGraphPassBuilder &builder) {
            builder.compute_shader_read(_panorama_texture);
            builder.compute_shader_write(_tmp_env_map);
        },
        [=](CommandBuffer *cmd) {
            capture_env_cube_map(cmd, _panorama_texture, _tmp_env_map);
        });

    Texture::generate_mipmap(_tmp_env_map, rg);

    rg.add_pass(
        [&](RenderGraphPassBuilder &builder) {
            builder.compute_shader_read(_tmp_env_map);
            builder.compute_shader_write(_irradiance_cube_map);
        },
        [=](CommandBuffer *cmd) {
            prefilter_irradiance(cmd, _tmp_env_map, _irradiance_cube_map);
        });
}

PanoramaSky::~PanoramaSky() = default;

PhysicalSky::PhysicalSky(Context *context)
    : _context(context), SkyBase(context) {}

SkyData::SkyData(Context *context) : _context(context) {
    _prefilter_env_pipeline =
        ComputePipeline::create(context, "IBL/PrefilterCubeMap.comp");
    _capture_cube_map_pipeline =
        ComputePipeline::create(context, "IBL/CaptureHDRToCubeMap.comp");
}

bool SkyData::cache_dirty() {
    // Default case
    if (_panorama_texture == nullptr) {
        assert(_irradiance_cube_map == nullptr);
        return false;
    }
    if (_panorama_dirty) {
        return true;
    }
    // The panorama texture does not change, but irradiance cube map not
    // initialized or size mismatch
    if (_irradiance_cube_map == nullptr) {
        return true;
    }
    return _irradiance_cube_map->info().extent.width !=
           _irradiance_cube_map_size;
}

SkyData::~SkyData() = default;

SkyData *SkyBase::data() {
    return _data.get();
}

SkyBase::SkyBase(Context *context) {
    _data = std::make_unique<SkyData>(context);
}

std::shared_ptr<PanoramaSky> upcast(const std::shared_ptr<IPanoramaSky> &sky) {
    return std::dynamic_pointer_cast<PanoramaSky>(sky);
}

std::shared_ptr<PhysicalSky> upcast(const std::shared_ptr<IPhysicalSky> &sky) {
    return std::dynamic_pointer_cast<PhysicalSky>(sky);
}

std::shared_ptr<SkyBase> upcast(const std::shared_ptr<ISky> &sky) {
    return std::dynamic_pointer_cast<SkyBase>(sky);
}
} // namespace ars::render::vk
