#include "RenderGraph.h"

namespace ars::render::vk {
void barrier(CommandBuffer *cmd,
             const std::vector<PassDependency> &src_deps,
             const std::vector<PassDependency> &dst_deps) {
    std::vector<VkImageMemoryBarrier> barriers{};
    barriers.reserve(src_deps.size() + dst_deps.size());

    VkPipelineStageFlags src_stage_mask = 0;
    VkPipelineStageFlags dst_stage_mask = 0;

    for (auto &dep : src_deps) {
        assert(dep.texture.get() != nullptr);

        src_stage_mask |= dep.stage_mask;

        VkImageMemoryBarrier b{VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
        b.subresourceRange = dep.texture->subresource_range();
        b.srcAccessMask = dep.access_mask;
        b.dstAccessMask = 0;

        // Maybe we should ignore dep.layout for src?
        assert(dep.layout == dep.texture->layout());

        // If there is only src dependency, do not change image layout
        b.oldLayout = dep.layout;
        b.newLayout = dep.layout;
        b.image = dep.texture->image();

        barriers.push_back(b);
    }

    auto find_src_barrier =
        [&](const PassDependency &dep) -> VkImageMemoryBarrier * {
        assert(dep.texture.get() != nullptr);

        for (auto &b : barriers) {
            if (b.image == dep.texture->image()) {
                return &b;
            }
        }
        return nullptr;
    };

    for (auto &dep : dst_deps) {
        dst_stage_mask |= dep.stage_mask;

        auto src_barrier = find_src_barrier(dep);
        if (src_barrier != nullptr) {
            auto &b = *src_barrier;
            b.dstAccessMask = dep.access_mask;
            b.newLayout = dep.layout;
        } else {
            VkImageMemoryBarrier b{VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
            b.subresourceRange = dep.texture->subresource_range();
            b.srcAccessMask = 0;
            b.dstAccessMask = dep.access_mask;
            b.oldLayout = dep.texture->layout();
            b.newLayout = dep.layout;
            b.image = dep.texture->image();

            barriers.push_back(b);
        }
    }

    cmd->PipelineBarrier(src_stage_mask,
                         dst_stage_mask,
                         0,
                         0,
                         nullptr,
                         0,
                         nullptr,
                         static_cast<uint32_t>(barriers.size()),
                         barriers.data());

    for (auto &dep : dst_deps) {
        dep.texture->assure_layout(dep.layout);
    }
}
} // namespace ars::render::vk
