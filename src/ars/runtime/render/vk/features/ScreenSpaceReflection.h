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
    explicit ScreenSpaceReflection(View *view);

    std::vector<PassDependency> src_dependencies() override;
    std::vector<PassDependency> dst_dependencies() override;
    void render(CommandBuffer *cmd) override;

  private:
    View *_view = nullptr;
};
} // namespace ars::render::vk