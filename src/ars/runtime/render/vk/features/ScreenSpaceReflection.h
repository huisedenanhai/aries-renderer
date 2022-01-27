#pragma once

#include "../Pipeline.h"
#include "../RenderGraph.h"
#include "../View.h"

namespace ars::render::vk {
class GenerateHierarchyZ : public IRenderGraphPass {
  public:
    explicit GenerateHierarchyZ(View *view);

    std::vector<PassDependency> src_dependencies() override;
    std::vector<PassDependency> dst_dependencies() override;
    void render(CommandBuffer *cmd) override;

  private:
    void init_hiz(CommandBuffer *cmd);
    void propagate_hiz(CommandBuffer *cmd);

    View *_view = nullptr;
    std::unique_ptr<ComputePipeline> _init_hiz_pipeline;
    std::unique_ptr<ComputePipeline> _propagate_hiz_pipeline;
};

class ScreenSpaceReflection : public IRenderGraphPass {
  public:
    ScreenSpaceReflection(View *view, NamedRT src_rt, NamedRT dst_rt);

    std::vector<PassDependency> src_dependencies() override;
    std::vector<PassDependency> dst_dependencies() override;
    void render(CommandBuffer *cmd) override;

  private:
    View *_view = nullptr;
    NamedRT _src_rt_name{};
    NamedRT _dst_rt_name{};

    std::unique_ptr<ComputePipeline> _ssr_pipeline;
};
} // namespace ars::render::vk