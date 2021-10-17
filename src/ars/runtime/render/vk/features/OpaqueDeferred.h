#pragma once

#include "../Pipeline.h"
#include "../Vulkan.h"

namespace ars::render::vk {
class View;

class OpaqueDeferred {
  public:
    explicit OpaqueDeferred(View *view);
    ~OpaqueDeferred();

    void render(CommandBuffer *cmd);

  private:
    void init_render_pass();
    void init_pipeline();

    View *_view = nullptr;
    VkRenderPass _render_pass = VK_NULL_HANDLE;
    std::unique_ptr<GraphicsPipeline> _base_color_pipeline{};
};
} // namespace ars::render::vk