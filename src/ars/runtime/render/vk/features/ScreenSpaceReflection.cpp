#include "ScreenSpaceReflection.h"
#include "../Context.h"
#include "../Effect.h"
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
    desc.commit(cmd, _init_hiz_pipeline.get());

    _init_hiz_pipeline->local_size().dispatch(
        cmd, hiz_extent.width, hiz_extent.height, 1);
}

void GenerateHierarchyZ::propagate_hiz(CommandBuffer *cmd) {
    ARS_PROFILER_SAMPLE_VK(cmd, "Propagate HiZ", 0xFF5291AA);
    auto hiz_buffer = _view->render_target(NamedRT_HiZBuffer);
    auto &hiz_info = hiz_buffer->info();

    _propagate_hiz_pipeline->bind(cmd);
    auto cur_level_width = hiz_info.extent.width;
    auto cur_level_height = hiz_info.extent.height;
    for (int i = 1; i < hiz_info.mip_levels; i++) {
        cur_level_width = calculate_next_mip_size(cur_level_width);
        cur_level_height = calculate_next_mip_size(cur_level_height);

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
        desc.commit(cmd, _propagate_hiz_pipeline.get());

        _propagate_hiz_pipeline->local_size().dispatch(
            cmd, cur_level_width, cur_level_height, 1);
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
    auto ctx = _view->context();
    _hiz_trace_pipeline = ComputePipeline::create(ctx, "SSR/HiZTraceRay.comp");
    _resolve_reflection_pipeline =
        ComputePipeline::create(ctx, "SSR/ResolveReflection.comp");
    _temporal_filter_pipeline =
        ComputePipeline::create(ctx, "SSR/TemporalFiltering.comp");

    alloc_hit_buffer();
    alloc_resolve_single_sample_buffer();
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
                int32_t frame_index;
                float unbiased_sampling;
                ARS_PADDING_FIELD(int32_t);
                ARS_PADDING_FIELD(int32_t);
                ARS_PADDING_FIELD(int32_t);
            };

            Param param{};
            param.P = _view->projection_matrix();
            param.I_P = glm::inverse(param.P);
            param.width = static_cast<int32_t>(dst_extent.width);
            param.height = static_cast<int32_t>(dst_extent.height);
            param.hiz_mip_count =
                static_cast<int32_t>(hiz_buffer->info().mip_levels);
            param.frame_index = _frame_index++;
            auto ssr_settings = _view->effect()->screen_space_reflection();
            param.unbiased_sampling =
                std::clamp(1.0f - ssr_settings->sampling_bias(), 0.0f, 1.0f);

            desc.set_buffer_data(1, 0, param);

            desc.commit(cmd, _hiz_trace_pipeline.get());

            _hiz_trace_pipeline->local_size().dispatch(cmd, dst_extent);
        });
}

void ScreenSpaceReflection::alloc_hit_buffer() {
    TextureCreateInfo info =
        TextureCreateInfo::sampled_2d(VK_FORMAT_R16G16B16A16_SFLOAT,
                                      1,
                                      1,
                                      1,
                                      VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
    info.usage |= VK_IMAGE_USAGE_STORAGE_BIT;
    info.min_filter = VK_FILTER_NEAREST;
    info.mag_filter = VK_FILTER_NEAREST;

    // Trace at half resolution
    _hit_buffer_id = _view->rt_manager()->alloc(info, 0.5f);
}

void ScreenSpaceReflection::resolve_reflection(RenderGraph &rg) {
    rg.add_pass(
        [&](RenderGraphPassBuilder &builder) {
            builder.compute_shader_read(NamedRT_GBuffer1);
            builder.compute_shader_read(NamedRT_GBuffer2);
            builder.compute_shader_read(NamedRT_Depth);
            builder.compute_shader_read(NamedRT_LinearColorHistory);
            builder.compute_shader_read(_hit_buffer_id);
            builder.compute_shader_write(_resolve_buffer_single_sample);
        },
        [=](CommandBuffer *cmd) {
            _resolve_reflection_pipeline->bind(cmd);
            auto reflect_color_image =
                _view->rt_manager()->get(_resolve_buffer_single_sample);
            auto cube_map = _view->environment_vk()->irradiance_cube_map_vk();
            auto dst_extent = reflect_color_image->info().extent;

            DescriptorEncoder desc{};
            desc.set_texture(0, 0, reflect_color_image.get());
            desc.set_texture(
                0, 1, _view->render_target(NamedRT_GBuffer1).get());
            desc.set_texture(
                0, 2, _view->render_target(NamedRT_GBuffer2).get());
            desc.set_texture(0, 3, _view->render_target(NamedRT_Depth).get());
            desc.set_texture(
                0, 4, _view->rt_manager()->get(_hit_buffer_id).get());
            desc.set_texture(
                0, 5, _view->render_target(NamedRT_LinearColorHistory).get());

            struct Param {
                int32_t width;
                int32_t height;
                int32_t frame_index;
                ARS_PADDING_FIELD(int32_t);
                glm::mat4 P;
                glm::mat4 I_P;
                glm::mat4 I_V;
                glm::mat4 reproject_IV_VP;
            };

            Param param{};
            param.width = static_cast<int32_t>(dst_extent.width);
            param.height = static_cast<int32_t>(dst_extent.height);
            param.frame_index = _frame_index;
            param.P = _view->projection_matrix();
            param.I_P = glm::inverse(param.P);
            param.I_V = glm::inverse(_view->view_matrix());
            param.reproject_IV_VP = _view->last_frame_projection_matrix() *
                                    _view->last_frame_view_matrix() * param.I_V;
            desc.set_buffer_data(1, 0, param);

            desc.commit(cmd, _resolve_reflection_pipeline.get());

            _resolve_reflection_pipeline->local_size().dispatch(cmd,
                                                                dst_extent);
        });
}

void ScreenSpaceReflection::alloc_resolve_single_sample_buffer() {
    auto info =
        TextureCreateInfo::sampled_2d(VK_FORMAT_R16G16B16A16_SFLOAT, 1, 1, 1);
    info.usage |= VK_IMAGE_USAGE_STORAGE_BIT;

    // Resolve at full resolution
    _resolve_buffer_single_sample = _view->rt_manager()->alloc(info);
}

void ScreenSpaceReflection::render(RenderGraph &rg) {
    trace_rays(rg);
    resolve_reflection(rg);
    temporal_filtering(rg);
}

void ScreenSpaceReflection::temporal_filtering(RenderGraph &rg) {
    rg.add_pass(
        [&](RenderGraphPassBuilder &builder) {
            builder.compute_shader_read(NamedRT_Depth);
            builder.compute_shader_read(NamedRT_ReflectionHistory);
            builder.compute_shader_read(_resolve_buffer_single_sample);
            builder.compute_shader_write(NamedRT_Reflection);
        },
        [=](CommandBuffer *cmd) {
            _temporal_filter_pipeline->bind(cmd);
            auto reflect_tex = _view->render_target(NamedRT_Reflection);
            auto dst_extent = reflect_tex->info().extent;

            DescriptorEncoder desc{};
            desc.set_texture(0, 0, reflect_tex.get());
            desc.set_texture(0, 1, _view->render_target(NamedRT_Depth).get());
            desc.set_texture(
                0, 2, _view->render_target(NamedRT_ReflectionHistory).get());
            desc.set_texture(
                0,
                3,
                _view->rt_manager()->get(_resolve_buffer_single_sample).get());

            struct Param {
                int width;
                int height;
                float blend_factor;
                ARS_PADDING_FIELD(int32_t);
                glm::mat4 I_P;
                glm::mat4 reproject_IV_VP;
            };

            Param param{};
            param.width = static_cast<int32_t>(dst_extent.width);
            param.height = static_cast<int32_t>(dst_extent.height);
            param.blend_factor = _frame_index == 0 ? 1 : 0.05f;
            param.I_P = glm::inverse(_view->projection_matrix());
            param.reproject_IV_VP = _view->last_frame_projection_matrix() *
                                    _view->last_frame_view_matrix() *
                                    glm::inverse(_view->view_matrix());
            desc.set_buffer_data(1, 0, param);

            desc.commit(cmd, _temporal_filter_pipeline.get());

            _temporal_filter_pipeline->local_size().dispatch(cmd, dst_extent);
        });
}
} // namespace ars::render::vk
