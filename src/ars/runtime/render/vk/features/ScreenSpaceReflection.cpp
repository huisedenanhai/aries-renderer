#include "ScreenSpaceReflection.h"
#include "../Profiler.h"

namespace ars::render::vk {
std::vector<PassDependency> GenerateHierarchyZ::src_dependencies() {
    std::vector<PassDependency> deps(2);
    {
        auto &d = deps[0];
        d.texture = _view->render_target(NamedRT_Depth);
        d.access_mask = VK_ACCESS_SHADER_READ_BIT;
        d.layout = VK_IMAGE_LAYOUT_GENERAL;
        d.stage_mask = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
    }
    {
        auto &d = deps[1];
        d.texture = _view->render_target(NamedRT_HiZBuffer);
        d.access_mask = VK_ACCESS_SHADER_WRITE_BIT;
        d.layout = VK_IMAGE_LAYOUT_GENERAL;
        d.stage_mask = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
    }
    return deps;
}

std::vector<PassDependency> GenerateHierarchyZ::dst_dependencies() {
    std::vector<PassDependency> deps(1);
    {
        auto &d = deps[0];
        d.texture = _view->render_target(NamedRT_HiZBuffer);
        d.access_mask = VK_ACCESS_SHADER_WRITE_BIT;
        d.layout = VK_IMAGE_LAYOUT_GENERAL;
        d.stage_mask = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
    }
    return deps;
}

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

std::vector<PassDependency> ScreenSpaceReflection::src_dependencies() {
    std::vector<PassDependency> deps{};
    auto add_dep = [&](NamedRT rt_name, VkAccessFlags access_mask) {
        PassDependency d{};
        d.texture = _view->render_target(rt_name);
        d.access_mask = access_mask;
        d.layout = VK_IMAGE_LAYOUT_GENERAL;
        d.stage_mask = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
        deps.push_back(d);
    };
    add_dep(NamedRT_HiZBuffer, VK_ACCESS_SHADER_READ_BIT);
    add_dep(_src_rt_name, VK_ACCESS_SHADER_READ_BIT);
    add_dep(_dst_rt_name, VK_ACCESS_SHADER_WRITE_BIT);
    add_dep(NamedRT_GBuffer0, VK_ACCESS_SHADER_READ_BIT);
    add_dep(NamedRT_GBuffer1, VK_ACCESS_SHADER_READ_BIT);
    add_dep(NamedRT_GBuffer2, VK_ACCESS_SHADER_READ_BIT);

    return deps;
}

std::vector<PassDependency> ScreenSpaceReflection::dst_dependencies() {
    std::vector<PassDependency> deps(1);
    {
        auto &d = deps[0];
        d.texture = _view->render_target(_dst_rt_name);
        d.access_mask = VK_ACCESS_SHADER_WRITE_BIT;
        d.layout = VK_IMAGE_LAYOUT_GENERAL;
        d.stage_mask = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
    }
    return deps;
}

void ScreenSpaceReflection::execute(CommandBuffer *cmd) {
    ARS_PROFILER_SAMPLE_VK(cmd, "Screen Space Reflection", 0xFF432134);
    auto src_color_tex = _view->render_target(_src_rt_name);
    auto dst_color_tex = _view->render_target(_dst_rt_name);
    auto hiz_buffer = _view->render_target(NamedRT_HiZBuffer);

    auto dst_extent = dst_color_tex->info().extent;
    _ssr_pipeline->bind(cmd);

    DescriptorEncoder desc{};
    desc.set_texture(0, 0, dst_color_tex.get());
    desc.set_texture(0, 1, hiz_buffer.get());
    desc.set_texture(0, 2, src_color_tex.get());
    desc.set_texture(0, 3, _view->render_target(NamedRT_GBuffer0).get());
    desc.set_texture(0, 4, _view->render_target(NamedRT_GBuffer1).get());
    desc.set_texture(0, 5, _view->render_target(NamedRT_GBuffer2).get());

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
    param.hiz_mip_count = static_cast<int32_t>(hiz_buffer->info().mip_levels);

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

    desc.commit(cmd, _ssr_pipeline.get());

    _ssr_pipeline->local_size().dispatch(cmd, dst_extent);
}

ScreenSpaceReflection::ScreenSpaceReflection(View *view,
                                             NamedRT src_rt,
                                             NamedRT dst_rt)
    : _view(view), _src_rt_name(src_rt), _dst_rt_name(dst_rt) {
    _ssr_pipeline = ComputePipeline::create(_view->context(),
                                            "SSR/ScreenSpaceReflection.comp");
}
} // namespace ars::render::vk
