#pragma once

#include "../Pipeline.h"
#include "../RenderPass.h"
#include "../View.h"
#include "../Vulkan.h"
#include <array>

namespace ars::render::vk {
class OpaqueDeferred {
  public:
    explicit OpaqueDeferred(View *view);
    ~OpaqueDeferred() = default;

    void render(CommandBuffer *cmd);

  private:
    void init_render_pass();
    void init_geometry_pass_pipeline();
    void init_shading_pass_pipeline();
    [[nodiscard]] std::array<NamedRT, 5> geometry_pass_rts() const;

    View *_view = nullptr;
    std::unique_ptr<RenderPass> _render_pass{};
    std::unique_ptr<GraphicsPipeline> _geometry_pass_pipeline{};
    std::unique_ptr<ComputePipeline> _shading_pass_pipeline{};
};
} // namespace ars::render::vk