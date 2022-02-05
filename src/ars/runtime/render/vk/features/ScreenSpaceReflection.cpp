#include "ScreenSpaceReflection.h"
#include "../Context.h"
#include "../Environment.h"
#include "../Lut.h"
#include "../Profiler.h"

namespace ars::render::vk {
void GenerateHierarchyZ::execute(CommandBuffer *cmd) {
    ARS_PROFILER_SAMPLE_VK(cmd, "Generate HiZ", 0xFF116622);
    init_hiz(cmd);
    propagate_hiz(cmd);
}

GenerateHierarchyZ::GenerateHierarchyZ(View *view) : _view(view) {
    auto ctx = _view->context();
    _init_hiz_pipeline = ComputePipeline::create(ctx, "SSR/InitHiZ.comp");
    _propagate_hiz_pipeline =
        ComputePipeline::create(ctx, "SSR/PropagateHiZ.comp");
}

void GenerateHierarchyZ::init_hiz(CommandBuffer *cmd) {
    ARS_PROFILER_SAMPLE_VK(cmd, "Init HiZ", 0xFF717312);
    auto depth_buffer = _view->render_target(NamedRT_Depth);
    auto hiz_buffer = _view->render_target(NamedRT_HiZBuffer);
    auto hiz_extent = hiz_buffer->info().extent;

    _init_hiz_pipeline->bind(cmd);
    DescriptorEncoder desc{};
    desc.set_texture(0, 0, depth_buffer.get());
    desc.set_texture(0, 1, hiz_buffer.get(), 0);

    struct Param {
        int32_t width;
        int32_t height;
    };
    Param param{};
    param.width = static_cast<int32_t>(hiz_extent.width);
    param.height = static_cast<int32_t>(hiz_extent.height);
    desc.set_buffer_data(1, 0, param);

    desc.commit(cmd, _init_hiz_pipeline.get());

    _init_hiz_pipeline->local_size().dispatch(
        cmd, hiz_extent.width, hiz_extent.height, 1);
}

void GenerateHierarchyZ::propagate_hiz(CommandBuffer *cmd) {
    ARS_PROFILER_SAMPLE_VK(cmd, "Propagate HiZ", 0xFF5291AA);
    auto hiz_buffer = _view->render_target(NamedRT_HiZBuffer);
    auto &hiz_info = hiz_buffer->info();

    _propagate_hiz_pipeline->bind(cmd);
    auto last_level_width = hiz_info.extent.width;
    auto last_level_height = hiz_info.extent.height;
    for (int i = 1; i < hiz_info.mip_levels; i++) {

        // Wait for last level initialized
        VkImageMemoryBarrier barrier{VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
        barrier.image = hiz_buffer->image();
        barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
        barrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
        barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        barrier.subresourceRange = hiz_buffer->subresource_range();
        barrier.subresourceRange.baseMipLevel = i - 1;
        barrier.subresourceRange.levelCount = 1;

        cmd->PipelineBarrier(VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                             VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                             0,
                             0,
                             nullptr,
                             0,
                             nullptr,
                             1,
                             &barrier);

        DescriptorEncoder desc{};
        desc.set_texture(0, 0, hiz_buffer.get(), i - 1);
        desc.set_texture(0, 1, hiz_buffer.get(), i);

        struct Param {
            int32_t last_level_width;
            int32_t last_level_height;
        };

        auto cur_level_width = calculate_next_mip_size(last_level_width);
        auto cur_level_height = calculate_next_mip_size(last_level_height);

        Param param{};
        param.last_level_width = static_cast<int32_t>(last_level_width);
        param.last_level_height = static_cast<int32_t>(last_level_height);
        desc.set_buffer_data(1, 0, param);
        desc.commit(cmd, _propagate_hiz_pipeline.get());

        _propagate_hiz_pipeline->local_size().dispatch(
            cmd, cur_level_width, cur_level_height, 1);

        // Update size
        last_level_width = cur_level_width;
        last_level_height = cur_level_height;
    }
}

void GenerateHierarchyZ::render(RenderGraph &rg) {
    rg.add_pass(
        [&](RenderGraphPassBuilder &builder) {
            builder.compute_shader_read(NamedRT_Depth);
            builder.compute_shader_write(NamedRT_HiZBuffer);
        },
        [this](CommandBuffer *cmd) { execute(cmd); });
}

ScreenSpaceReflection::ScreenSpaceReflection(View *view) : _view(view) {
    _hiz_trace_pipeline =
        ComputePipeline::create(_view->context(), "SSR/HiZTraceRay.comp");
    _resolve_reflection =
        ComputePipeline::create(_view->context(), "SSR/ResolveReflection.comp");

    alloc_hit_buffer();
}

void ScreenSpaceReflection::trace_rays(RenderGraph &rg) {
    rg.add_pass(
        [&](RenderGraphPassBuilder &builder) {
            builder.compute_shader_read(NamedRT_HiZBuffer);
            builder.compute_shader_read(NamedRT_GBuffer1);
            builder.compute_shader_read(NamedRT_GBuffer2);
            builder.compute_shader_write(_hit_buffer_id);
        },
        [=](CommandBuffer *cmd) {
            ARS_PROFILER_SAMPLE_VK(cmd, "SSR Trace", 0xFF432134);
            auto hit_buffer = _view->rt_manager()->get(_hit_buffer_id);
            auto hiz_buffer = _view->render_target(NamedRT_HiZBuffer);

            auto dst_extent = hit_buffer->info().extent;

            _hiz_trace_pipeline->bind(cmd);

            DescriptorEncoder desc{};
            desc.set_texture(0, 0, hit_buffer.get());
            desc.set_texture(0, 1, hiz_buffer.get());
            desc.set_texture(
                0, 2, _view->render_target(NamedRT_GBuffer1).get());
            desc.set_texture(
                0, 3, _view->render_target(NamedRT_GBuffer2).get());

            struct Param {
                glm::mat4 P;
                glm::mat4 I_P;
                int32_t width;
                int32_t height;
                int32_t hiz_mip_count;
                ARS_PADDING_FIELD(uint32_t);
            };

            Param param{};
            param.P = _view->projection_matrix();
            param.I_P = glm::inverse(param.P);
            param.width = static_cast<int32_t>(dst_extent.width);
            param.height = static_cast<int32_t>(dst_extent.height);
            param.hiz_mip_count =
                static_cast<int32_t>(hiz_buffer->info().mip_levels);

            desc.set_buffer_data(1, 0, param);

            std::vector<glm::vec2> mip_sizes{};
            mip_sizes.reserve(param.hiz_mip_count);

            uint32_t mip_width = hiz_buffer->info().extent.width;
            uint32_t mip_height = hiz_buffer->info().extent.height;
            for (int i = 0; i < param.hiz_mip_count; i++) {
                mip_sizes.emplace_back(mip_width, mip_height);
                mip_width = calculate_next_mip_size(mip_width);
                mip_height = calculate_next_mip_size(mip_height);
            }

            desc.set_buffer_data(
                1, 1, mip_sizes.data(), mip_sizes.size() * sizeof(glm::vec2));

            desc.commit(cmd, _hiz_trace_pipeline.get());

            _hiz_trace_pipeline->local_size().dispatch(cmd, dst_extent);
        });
}

void ScreenSpaceReflection::alloc_hit_buffer() {
    RenderTargetInfo info{};
    auto &tex = info.texture =
        TextureCreateInfo::sampled_2d(VK_FORMAT_R16G16B16A16_SFLOAT, 1, 1, 1);
    tex.usage |= VK_IMAGE_USAGE_STORAGE_BIT;
    tex.min_filter = VK_FILTER_NEAREST;
    tex.mag_filter = VK_FILTER_NEAREST;

    // Trace at half resolution
    _hit_buffer_id = _view->rt_manager()->alloc(info, 0.5f);
}

void ScreenSpaceReflection::resolve_reflection(RenderGraph &rg) {
    rg.add_pass(
        [&](RenderGraphPassBuilder &builder) {
            builder.compute_shader_read(_hit_buffer_id);
            builder.compute_shader_read(NamedRT_GBuffer0);
            builder.compute_shader_read(NamedRT_GBuffer1);
            builder.compute_shader_read(NamedRT_GBuffer2);
            builder.compute_shader_read(NamedRT_Depth);
            builder.compute_shader_write(NamedRT_Reflection);
        },
        [=](CommandBuffer *cmd) {
            _resolve_reflection->bind(cmd);
            auto reflect_color_image = _view->render_target(NamedRT_Reflection);
            auto hit_buffer = _view->rt_manager()->get(_hit_buffer_id);

            auto dst_extent = reflect_color_image->info().extent;

            DescriptorEncoder desc{};
            desc.set_texture(0, 0, reflect_color_image.get());
            desc.set_texture(0, 1, hit_buffer.get());
            desc.set_texture(
                0, 2, _view->render_target(NamedRT_GBuffer0).get());
            desc.set_texture(
                0, 3, _view->render_target(NamedRT_GBuffer1).get());
            desc.set_texture(
                0, 4, _view->render_target(NamedRT_GBuffer2).get());
            desc.set_texture(0, 5, _view->render_target(NamedRT_Depth).get());
            desc.set_texture(0, 6, _view->context()->lut()->brdf_lut().get());
            desc.set_texture(
                0, 7, _view->environment_vk()->irradiance_cube_map_vk().get());

            struct Param {
                int32_t width;
                int32_t height;
                ARS_PADDING_FIELD(float);
                ARS_PADDING_FIELD(float);
                glm::mat4 I_P;
                glm::mat4 I_V;
                glm::vec3 env_radiance_factor;
                int32_t cube_map_mip_count;
            };

            Param param{};
            param.width = static_cast<int32_t>(dst_extent.width);
            param.height = static_cast<int32_t>(dst_extent.height);
            param.I_P = glm::inverse(_view->projection_matrix());
            param.I_V = glm::inverse(_view->view_matrix());
            param.env_radiance_factor = _view->environment_vk()->radiance();
            param.cube_map_mip_count =
                static_cast<int32_t>(_view->environment_vk()
                                         ->irradiance_cube_map_vk()
                                         ->info()
                                         .mip_levels);
            desc.set_buffer_data(1, 0, param);

            desc.commit(cmd, _resolve_reflection.get());

            _resolve_reflection->local_size().dispatch(cmd, dst_extent);
        });
}
} // namespace ars::render::vk
