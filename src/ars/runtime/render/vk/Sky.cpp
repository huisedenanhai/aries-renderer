#include "Sky.h"
#include "Context.h"
#include "Pipeline.h"
#include "Profiler.h"
#include "RenderGraph.h"

namespace ars::render::vk {
Handle<Texture> SkyData::irradiance_cube_map() {
    if (_irradiance_cube_map == nullptr) {
        return _context->default_texture_vk(DefaultTexture::WhiteCubeMap);
    }
    return _irradiance_cube_map;
}

PanoramaSky::PanoramaSky(Context *context)
    : _context(context), SkyBase(context) {
    data()->set_prefilter_bias(3.0);
    data()->set_prefilter_sample_count(256);
}

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

void ImageBasedLighting::capture_panorama_to_cube_map(
    CommandBuffer *cmd,
    const Handle<Texture> &panorama_tex,
    const Handle<Texture> &env_cube_map) {
    assert(panorama_tex != nullptr);
    assert(env_cube_map != nullptr);

    auto size = static_cast<int32_t>(env_cube_map->info().extent.width);
    _capture_cube_map_pipeline->bind(cmd);

    DescriptorEncoder desc{};
    desc.set_texture(0, 0, panorama_tex.get());
    desc.set_texture(0, 1, env_cube_map.get());
    desc.commit(cmd, _capture_cube_map_pipeline.get());

    _capture_cube_map_pipeline->local_size().dispatch(cmd, size, size, 6);
}

void ImageBasedLighting::prefilter_irradiance(
    CommandBuffer *cmd,
    const Handle<Texture> &env_cube_map,
    const Handle<Texture> &irradiance_cube_map,
    int sample_count,
    float bias) {
    assert(env_cube_map != nullptr);
    assert(irradiance_cube_map != nullptr);

    _prefilter_env_pipeline->bind(cmd);
    {
        DescriptorEncoder desc{};
        desc.set_texture(0, 0, env_cube_map.get());
        desc.commit(cmd, _prefilter_env_pipeline.get());
    }

    uint32_t mip_count = irradiance_cube_map->info().mip_levels;
    uint32_t size = irradiance_cube_map->info().extent.width;

    for (int i = 0; i < mip_count; i++) {
        struct Param {
            int32_t size;
            float roughness;
            int32_t mip_level;
            int32_t radiance_base_resolution;
            float bias;
            int32_t sample_count;
        };

        Param param{};
        param.size = static_cast<int32_t>(size);
        param.roughness = static_cast<float>(i) /
                          static_cast<float>(std::max(1u, mip_count - 1));
        param.mip_level = i;
        param.radiance_base_resolution =
            static_cast<int32_t>(env_cube_map->info().extent.width);
        param.bias = bias;
        param.sample_count = sample_count;

        DescriptorEncoder desc{};
        desc.set_buffer_data(1, 0, param);
        desc.set_texture(1, 1, irradiance_cube_map.get(), i);
        desc.commit(cmd, _prefilter_env_pipeline.get());

        _prefilter_env_pipeline->local_size().dispatch(cmd, size, size, 6);

        size = calculate_next_mip_size(size);
    }
}

ImageBasedLighting::ImageBasedLighting(Context *context) : _context(context) {
    _prefilter_env_pipeline =
        ComputePipeline::create(context, "IBL/PrefilterCubeMap.comp");
    _capture_cube_map_pipeline =
        ComputePipeline::create(context, "IBL/CaptureHDRToCubeMap.comp");
}

ImageBasedLighting::~ImageBasedLighting() = default;

Handle<Texture> SkyData::panorama() {
    if (_panorama_texture == nullptr) {
        return _context->default_texture_vk(DefaultTexture::White);
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
        _dirty = false;
        return;
    }

    _panorama_texture = hdr;
    _dirty = true;
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
    _dirty = false;

    auto ibl = _context->ibl();
    assert(_panorama_texture != nullptr);
    rg.add_pass(
        [&](RenderGraphPassBuilder &builder) {
            builder.compute_shader_read(_panorama_texture);
            builder.compute_shader_write(_tmp_env_map);
        },
        [=](CommandBuffer *cmd) {
            ARS_PROFILER_SAMPLE_VK(cmd, "Capture Env Cube Map", 0xFF017411);
            ibl->capture_panorama_to_cube_map(
                cmd, _panorama_texture, _tmp_env_map);
        });

    Texture::generate_mipmap(_tmp_env_map, rg);

    rg.add_pass(
        [&](RenderGraphPassBuilder &builder) {
            builder.compute_shader_read(_tmp_env_map);
            builder.compute_shader_write(_irradiance_cube_map);
        },
        [=](CommandBuffer *cmd) {
            ARS_PROFILER_SAMPLE_VK(cmd, "Prefilter Irradiance", 0xFF87411A);
            ibl->prefilter_irradiance(cmd,
                                      _tmp_env_map,
                                      _irradiance_cube_map,
                                      _prefilter_sample_count,
                                      _prefilter_bias);
        });
}

PanoramaSky::~PanoramaSky() = default;

namespace {
struct AtmosphereSettings {
    float bottom_radius;
    float top_radius;
    float mie_scattering;
    float mie_absorption;
    glm::vec3 rayleigh_scattering;
    float rayleigh_altitude;
    glm::vec3 ozone_absorption;
    float mie_altitude;
    float ozone_altitude;
    float ozone_thickness;
};
} // namespace

PhysicalSky::PhysicalSky(Context *context)
    : _context(context), SkyBase(context) {
    _physical_panorama_pipeline = ComputePipeline::create(
        _context, "Atmosphere/PhysicalSkyPanorama.comp");

    init_atmosphere_settings_buffer();
    init_textures();
}

void PhysicalSky::render(RenderGraph &rg) {
    auto panorama = data()->panorama();
    rg.add_pass(
        [&](RenderGraphPassBuilder &builder) {
            builder.compute_shader_write(panorama);
        },
        [=](CommandBuffer *cmd) {
            _physical_panorama_pipeline->bind(cmd);

            DescriptorEncoder desc{};
            desc.set_texture(0, 0, panorama.get());
            desc.commit(cmd, _physical_panorama_pipeline.get());

            _physical_panorama_pipeline->local_size().dispatch(
                cmd, panorama->info().extent);
        });
    data()->mark_dirty();
    data()->update_cache(rg);
}

void PhysicalSky::init_atmosphere_settings_buffer() {
    _atmosphere_settings_buffer =
        _context->create_buffer(sizeof(AtmosphereSettings),
                                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                VMA_MEMORY_USAGE_CPU_TO_GPU);

    AtmosphereSettings atmosphere{};
    atmosphere.bottom_radius = 6360.0f;
    atmosphere.top_radius = 6460.0f;
    atmosphere.mie_scattering = 3.996e-3f;
    atmosphere.mie_absorption = 4.40e-3f;
    atmosphere.rayleigh_scattering = glm::vec3(5.802, 13.558, 33.1) * 1.0e-3f;
    atmosphere.rayleigh_altitude = 8.0f;
    atmosphere.ozone_absorption = glm::vec3(0.650, 1.881, 0.085) * 1.0e-3f;
    atmosphere.mie_altitude = 1.2f;
    atmosphere.ozone_altitude = 25.0f;
    atmosphere.ozone_thickness = 30.0f;

    _atmosphere_settings_buffer->set_data(&atmosphere, 0, 1);
}

void PhysicalSky::init_textures() {
    data()->set_irradiance_cube_map_size(64);

    auto panorama_info =
        TextureCreateInfo::sampled_2d(VK_FORMAT_B10G11R11_UFLOAT_PACK32,
                                      256,
                                      128,
                                      1,
                                      VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
    panorama_info.usage |= VK_IMAGE_USAGE_STORAGE_BIT;
    data()->set_panorama(_context->create_texture(panorama_info));
    data()->set_prefilter_sample_count(64);
}

PhysicalSky::~PhysicalSky() = default;

SkyData::SkyData(Context *context) : _context(context) {}

bool SkyData::cache_dirty() {
    // Default case
    if (_panorama_texture == nullptr) {
        assert(_irradiance_cube_map == nullptr);
        return false;
    }
    if (_dirty) {
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

glm::vec3 SkyData::color() const {
    return _color;
}

void SkyData::set_color(glm::vec3 color) {
    _color = color;
}

float SkyData::strength() const {
    return _strength;
}

void SkyData::set_strength(float strength) {
    _strength = strength;
}

glm::vec3 SkyData::radiance() const {
    return color() * strength();
}

void SkyData::mark_dirty(bool dirty) {
    _dirty = dirty;
}

float SkyData::prefilter_bias() const {
    return _prefilter_bias;
}

void SkyData::set_prefilter_bias(float bias) {
    _prefilter_bias = bias;
    _dirty = true;
}

int32_t SkyData::prefilter_sample_count() const {
    return _prefilter_sample_count;
}

void SkyData::set_prefilter_sample_count(int32_t sample_count) {
    _prefilter_sample_count = sample_count;
    _dirty = true;
}

SkyData::~SkyData() = default;

SkyData *SkyBase::data() {
    return _data.get();
}

SkyBase::SkyBase(Context *context) {
    _data = std::make_unique<SkyData>(context);
}

void SkyBase::render(RenderGraph &rg) {
    data()->update_cache(rg);
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
