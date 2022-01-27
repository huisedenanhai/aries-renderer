#include "ScreenSpaceReflection.h"
#include "../Profiler.h"

namespace ars::render::vk {
std::vector<PassDependency> GenerateHierarchyZ::src_dependencies() {
    return {{
                .texture = _view->render_target(NamedRT_Depth),
                .access_mask = VK_ACCESS_SHADER_READ_BIT,
                .layout = VK_IMAGE_LAYOUT_GENERAL,
                .stage_mask = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
            },
            {
                .texture = _view->render_target(NamedRT_HiZBuffer),
                .access_mask = VK_ACCESS_SHADER_WRITE_BIT,
                .layout = VK_IMAGE_LAYOUT_GENERAL,
                .stage_mask = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
            }};
}

std::vector<PassDependency> GenerateHierarchyZ::dst_dependencies() {
    return {{
        .texture = _view->render_target(NamedRT_HiZBuffer),
        .access_mask = VK_ACCESS_SHADER_WRITE_BIT,
        .layout = VK_IMAGE_LAYOUT_GENERAL,
        .stage_mask = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
    }};
}

void GenerateHierarchyZ::render(CommandBuffer *cmd) {
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
    auto level_width = hiz_info.extent.width;
    auto level_height = hiz_info.extent.height;
    for (int i = 1; i < hiz_info.mip_levels; i++) {
        // Update size
        level_width = calculate_next_mip_size(hiz_info.extent.width);
        level_height = calculate_next_mip_size(hiz_info.extent.height);

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
            int32_t cur_level_width;
            int32_t cur_level_height;
        };
        Param param{};
        param.cur_level_width = static_cast<int32_t>(level_width);
        param.cur_level_height = static_cast<int32_t>(level_height);
        desc.set_buffer_data(1, 0, param);

        _propagate_hiz_pipeline->local_size().dispatch(
            cmd, level_width, level_height, 1);
    }
}

std::vector<PassDependency> ScreenSpaceReflection::src_dependencies() {
    return IRenderGraphPass::src_dependencies();
}

std::vector<PassDependency> ScreenSpaceReflection::dst_dependencies() {
    return IRenderGraphPass::dst_dependencies();
}

void ScreenSpaceReflection::render(CommandBuffer *cmd) {}

ScreenSpaceReflection::ScreenSpaceReflection(View *view) : _view(view) {}
} // namespace ars::render::vk
