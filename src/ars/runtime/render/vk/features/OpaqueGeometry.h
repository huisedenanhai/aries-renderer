#pragma once

#include "../Pipeline.h"
#include "../RenderGraph.h"
#include "../RenderPass.h"
#include "../View.h"
#include "../Vulkan.h"
#include <array>

namespace ars::render::vk {
class OpaqueGeometry : public IRenderGraphPass {
  public:
    explicit OpaqueGeometry(View *view);

    void execute(CommandBuffer *cmd) override;
    [[nodiscard]] std::vector<PassDependency> dst_dependencies() override;

  private:
    void init_render_pass();
    void init_pipeline();
    [[nodiscard]] static std::array<NamedRT, 5> geometry_pass_rt_names();

    View *_view = nullptr;
    std::unique_ptr<RenderPass> _render_pass{};
    std::unique_ptr<GraphicsPipeline> _pipeline{};
};
} // namespace ars::render::vk