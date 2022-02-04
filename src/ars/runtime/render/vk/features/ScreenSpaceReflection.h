#pragma once

#include "../Pipeline.h"
#include "../RenderGraph.h"
#include "../View.h"

namespace ars::render::vk {
class GenerateHierarchyZ {
  public:
    explicit GenerateHierarchyZ(View *view);

    void render(RenderGraph &rg);

  private:
    void execute(CommandBuffer *cmd);
    void init_hiz(CommandBuffer *cmd);
    void propagate_hiz(CommandBuffer *cmd);

    View *_view = nullptr;
    std::unique_ptr<ComputePipeline> _init_hiz_pipeline;
    std::unique_ptr<ComputePipeline> _propagate_hiz_pipeline;
};

class ScreenSpaceReflection {
  public:
    explicit ScreenSpaceReflection(View *view);

    void render(RenderGraph &rg, NamedRT src_rt, NamedRT dst_rt);

  private:
    void execute(CommandBuffer *cmd, NamedRT src_rt, NamedRT dst_rt);

    View *_view = nullptr;
    std::unique_ptr<ComputePipeline> _ssr_pipeline;
};
} // namespace ars::render::vk