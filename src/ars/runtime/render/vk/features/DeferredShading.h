#pragma once

#include "../Pipeline.h"
#include "../RenderGraph.h"
#include "../View.h"

namespace ars::render::vk {
class DeferredShading : public IRenderGraphPass {
  public:
    DeferredShading(View *view, NamedRT final_color_rt);

    std::vector<PassDependency> src_dependencies() override;
    void render(CommandBuffer *cmd) override;
    std::vector<PassDependency> dst_dependencies() override;

  private:
    void init_pipeline();

    View *_view = nullptr;
    NamedRT _final_color_rt{};
    std::unique_ptr<ComputePipeline> _pipeline{};
};
} // namespace ars::render::vk