#pragma once

#include "../Pipeline.h"
#include "../RenderGraph.h"
#include "../View.h"

namespace ars::render::vk {
class ToneMapping : public IRenderGraphPass {
  public:
    ToneMapping(View *view, NamedRT src_rt, NamedRT dst_rt);

    void execute(CommandBuffer *cmd) override;

    std::vector<PassDependency> src_dependencies() override;

  private:
    void init_pipeline();

    View *_view = nullptr;
    NamedRT _src_rt_name{};
    NamedRT _dst_rt_name{};

    std::unique_ptr<ComputePipeline> _pipeline{};
};
} // namespace ars::render::vk