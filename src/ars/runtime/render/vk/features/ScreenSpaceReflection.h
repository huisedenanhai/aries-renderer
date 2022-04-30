#pragma once

#include "../Effect.h"
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

    void render(RenderGraph &rg);

  private:
    void render(RenderGraph &rg, bool diffuse);
    void trace_rays(RenderGraph &rg,
                    IScreenSpaceReflectionEffect *settings,
                    bool diffuse);
    void resolve_reflection(RenderGraph &rg,
                            IScreenSpaceReflectionEffect *settings,
                            bool diffuse);
    void temporal_filtering(RenderGraph &rg,
                            NamedRT history_rt,
                            NamedRT result_rt,
                            bool history_valid);

    void alloc_hit_buffer();
    void alloc_resolve_single_sample_buffer();

    View *_view = nullptr;
    bool _reflection_history_valid[2]{};
    int32_t _frame_index = 0;
    std::unique_ptr<ComputePipeline> _hiz_trace_pipeline[2];
    std::unique_ptr<ComputePipeline> _resolve_reflection_pipeline[2];
    std::unique_ptr<ComputePipeline> _temporal_filter_pipeline;
    RenderTargetId _hit_buffer_id;
    RenderTargetId _resolve_buffer_single_sample;
};
} // namespace ars::render::vk