#pragma once

#include "../Pipeline.h"
#include "../RenderGraph.h"
#include "../View.h"

namespace ars::render::vk {
class DeferredShading {
  public:
    DeferredShading(View *view, NamedRT final_color_rt);

    [[nodiscard]] std::vector<PassDependency> src_dependencies();
    void render(CommandBuffer *cmd);
    [[nodiscard]] std::vector<PassDependency> dst_dependencies();

  private:
    void init_pipeline();

    View *_view = nullptr;
    NamedRT _final_color_rt{};
    std::unique_ptr<ComputePipeline> _pipeline{};
};
} // namespace ars::render::vk