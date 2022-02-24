#include "Sky.h"
#include "Context.h"
#include "Effect.h"
#include "Pipeline.h"
#include "Profiler.h"
#include "RenderGraph.h"
#include "Scene.h"

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
    float ground_albedo;
    float mie_g;
};
} // namespace

PhysicalSky::PhysicalSky(Context *context)
    : _context(context), SkyBase(context) {
    init_pipelines();
    init_atmosphere_settings_buffer();
    init_textures();
}

void PhysicalSky::update(View *view, RenderGraph &rg) {
    update_transmittance_lut(rg);
    update_multi_scattering_lut(rg);
    update_sky_view(view, rg);

    data()->mark_dirty();
    data()->update_cache(rg);
}

void PhysicalSky::init_atmosphere_settings_buffer() {
    _atmosphere_settings_buffer =
        _context->create_buffer(sizeof(AtmosphereSettings),
                                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                VMA_MEMORY_USAGE_CPU_TO_GPU);

    // Initialize with earth data
    // Length unit is km
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
    atmosphere.ground_albedo = 0.3f;
    atmosphere.mie_g = 0.8f;

    _atmosphere_settings_buffer->set_data(atmosphere);
}

void PhysicalSky::init_textures() {
    data()->set_irradiance_cube_map_size(64);

    auto panorama_info =
        TextureCreateInfo::sampled_2d(VK_FORMAT_B10G11R11_UFLOAT_PACK32,
                                      200,
                                      100,
                                      1,
                                      VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
    panorama_info.usage |= VK_IMAGE_USAGE_STORAGE_BIT;

    data()->set_panorama(_context->create_texture(panorama_info));
    data()->set_prefilter_sample_count(64);

    auto trans_lut_info = TextureCreateInfo::sampled_2d(
        VK_FORMAT_A2B10G10R10_UNORM_PACK32,
        256,
        64,
        1,
        // Ray hits ground will sample the black border, so the Lut already
        // encodes planet shadow info, no need for manual intersection
        // calculation when calculating shadow factor.
        VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER);
    trans_lut_info.usage |= VK_IMAGE_USAGE_STORAGE_BIT;
    _transmittance_lut = _context->create_texture(trans_lut_info);

    auto multi_scatter_lut_info =
        TextureCreateInfo::sampled_2d(VK_FORMAT_R16G16B16A16_SFLOAT,
                                      32,
                                      32,
                                      1,
                                      VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER);
    multi_scatter_lut_info.usage |= VK_IMAGE_USAGE_STORAGE_BIT;
    _multi_scattering_lut = _context->create_texture(multi_scatter_lut_info);
}

void PhysicalSky::update_transmittance_lut(RenderGraph &rg) {
    rg.add_pass(
        [&](RenderGraphPassBuilder &builder) {
            builder.compute_shader_write(_transmittance_lut);
        },
        [=](CommandBuffer *cmd) {
            _transmittance_lut_pipeline->bind(cmd);

            DescriptorEncoder desc{};
            desc.set_texture(0, 0, _transmittance_lut.get());
            desc.set_buffer(1, 0, _atmosphere_settings_buffer.get());
            desc.commit(cmd, _transmittance_lut_pipeline.get());

            _transmittance_lut_pipeline->local_size().dispatch(
                cmd, _transmittance_lut->info().extent);
        });
}

void PhysicalSky::init_pipelines() {
    _sky_view_lut_pipeline =
        ComputePipeline::create(_context, "Atmosphere/SkyViewLut.comp");
    _transmittance_lut_pipeline =
        ComputePipeline::create(_context, "Atmosphere/TransmittanceLut.comp");
    _multi_scattering_lut_pipeline =
        ComputePipeline::create(_context, "Atmosphere/MultiScatteringLut.comp");
}

void PhysicalSky::update_sky_view(View *view, RenderGraph &rg) {
    auto panorama = data()->panorama();
    rg.add_pass(
        [&](RenderGraphPassBuilder &builder) {
            builder.compute_shader_read(_transmittance_lut);
            builder.compute_shader_read(_multi_scattering_lut);
            builder.compute_shader_write(panorama);
        },
        [=](CommandBuffer *cmd) {
            _sky_view_lut_pipeline->bind(cmd);

            DescriptorEncoder desc{};
            desc.set_texture(0, 0, panorama.get());
            desc.set_texture(0, 1, _transmittance_lut.get());
            desc.set_texture(0, 2, _multi_scattering_lut.get());
            desc.set_buffer(1, 0, _atmosphere_settings_buffer.get());

            struct Param {
                glm::vec3 sun_dir;
                ARS_PADDING_FIELD(float);
                glm::vec3 sun_radiance;
                ARS_PADDING_FIELD(float);
            };

            Param param{};
            auto scene = view->scene_vk();
            if (scene->sun_id.valid()) {
                auto sun_xform =
                    scene->directional_lights.get<math::XformTRS<float>>(
                        scene->sun_id);
                auto sun_light =
                    scene->directional_lights.get<Light>(scene->sun_id);

                param.sun_dir = -glm::normalize(sun_xform.forward());
                param.sun_radiance = sun_light.color * sun_light.intensity;
            }
            desc.set_buffer_data(1, 1, param);

            desc.commit(cmd, _sky_view_lut_pipeline.get());

            _sky_view_lut_pipeline->local_size().dispatch(
                cmd, panorama->info().extent);
        });
}

void PhysicalSky::update_multi_scattering_lut(RenderGraph &rg) {
    rg.add_pass(
        [&](RenderGraphPassBuilder &builder) {
            builder.compute_shader_read(_transmittance_lut);
            builder.compute_shader_write(_multi_scattering_lut);
        },
        [=](CommandBuffer *cmd) {
            _multi_scattering_lut_pipeline->bind(cmd);

            DescriptorEncoder desc{};
            desc.set_texture(0, 0, _multi_scattering_lut.get());
            desc.set_texture(0, 1, _transmittance_lut.get());
            desc.set_buffer(1, 0, _atmosphere_settings_buffer.get());
            desc.commit(cmd, _multi_scattering_lut_pipeline.get());

            _multi_scattering_lut_pipeline->local_size().dispatch(
                cmd, _multi_scattering_lut->info().extent);
        });
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
    _shade_background_panorama_pipeline =
        ComputePipeline::create(context, "Shading/BackgroundPanorama.comp");
}

void SkyBase::update(View *view, RenderGraph &rg) {
    data()->update_cache(rg);
}

void SkyBase::render_background(View *view, RenderGraph &rg) {
    auto background = view->effect_vk()->background_vk();
    rg.add_pass(
        [&](RenderGraphPassBuilder &builder) {
            builder.compute_shader_write(NamedRT_LinearColor, false);
            builder.compute_shader_read(NamedRT_Depth);

            if (background->mode() == BackgroundMode::Sky) {
                builder.compute_shader_read(data()->panorama());
            }
        },
        [=](CommandBuffer *cmd) {
            auto background = view->effect_vk()->background_vk();
            auto final_rt = view->render_target(NamedRT_LinearColor);
            auto ctx = view->context();

            _shade_background_panorama_pipeline->bind(cmd);

            DescriptorEncoder desc{};
            desc.set_texture(0, 0, final_rt.get());
            desc.set_texture(0, 1, view->render_target(NamedRT_Depth).get());

            if (background->mode() == BackgroundMode::Sky) {
                desc.set_texture(0, 2, data()->panorama().get());
            } else {
                desc.set_texture(
                    0, 2, ctx->default_texture_vk(DefaultTexture::White).get());
            }

            struct Param {
                glm::vec3 background_factor;
            };

            Param param{};
            if (background->mode() == BackgroundMode::Sky) {
                param.background_factor = data()->radiance();
            } else {
                param.background_factor = background->radiance();
            }

            auto v_matrix = view->view_matrix();
            auto p_matrix = view->projection_matrix();

            desc.set_buffer_data(1, 0, param);
            desc.set_buffer(1, 1, view->transform_buffer().get());

            desc.commit(cmd, _shade_background_panorama_pipeline.get());

            _shade_background_panorama_pipeline->local_size().dispatch(
                cmd, final_rt->info().extent);
        });
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
