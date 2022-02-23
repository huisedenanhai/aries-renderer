#pragma once

#include "../Pipeline.h"
#include "../RenderGraph.h"
#include "../View.h"

namespace ars::render::vk {
class DeferredShading {
  public:
    explicit DeferredShading(View *view);

    void render(RenderGraph &rg);

  private:
    void execute(CommandBuffer *cmd);
    void init_pipeline();

    View *_view = nullptr;
    std::unique_ptr<ComputePipeline> _pipeline{};
};
} // namespace ars::render::vk