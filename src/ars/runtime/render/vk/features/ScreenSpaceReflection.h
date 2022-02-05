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

    void trace_rays(RenderGraph &rg);

    void resolve_reflection(RenderGraph &rg);

  private:
    void alloc_hit_buffer();

    View *_view = nullptr;
    std::unique_ptr<ComputePipeline> _hiz_trace_pipeline;
    std::unique_ptr<ComputePipeline> _resolve_reflection;
    RenderTargetId _hit_buffer_id;
};
} // namespace ars::render::vk