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

    // Textures in the src vector should be unique, so does the dst vector
    // Textures should be not null
    static void barrier(CommandBuffer *cmd,
                        const std::vector<PassDependency> &src_deps,
                        const std::vector<PassDependency> &dst_deps);
};

class IRenderGraphPass {
  public:
    virtual ~IRenderGraphPass() = default;

    [[nodiscard]] virtual std::vector<PassDependency> src_dependencies() {
        return {};
    }

    [[nodiscard]] virtual std::vector<PassDependency> dst_dependencies() {
        return {};
    }

    virtual void render(CommandBuffer *cmd) = 0;
};

class RenderGraph {
    // TODO
};
} // namespace ars::render::vk