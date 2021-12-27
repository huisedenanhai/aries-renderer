#pragma once

#include "Texture.h"
#include <vector>

namespace ars::render::vk {
class Texture;

struct PassDependency {
    Handle<Texture> texture{};
    VkAccessFlags access_mask{};
    VkImageLayout layout{};
    VkPipelineStageFlags stage_mask{};
};

// Textures in the src vector should be unique, so does the dst vector
// Textures should be not null
void barrier(CommandBuffer *cmd,
             const std::vector<PassDependency> &src_deps,
             const std::vector<PassDependency> &dst_deps);

class RenderGraph {
    // TODO
};
} // namespace ars::render::vk