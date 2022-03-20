#pragma once

#include "../Pipeline.h"
#include "../RenderGraph.h"
#include "../RenderPass.h"
#include "../View.h"
#include "../Vulkan.h"
#include <array>

namespace ars::render::vk {
class OpaqueGeometry {
  public:
    explicit OpaqueGeometry(View *view);

    void render(RenderGraph &rg);
    void execute(CommandBuffer *cmd);

  private:
    void init_pipeline();
    SubpassInfo render_pass();
    [[nodiscard]] static std::array<NamedRT, 5> geometry_pass_rt_names();

    View *_view = nullptr;
    std::unique_ptr<GraphicsPipeline> _pipeline{};
};
} // namespace ars::render::vk