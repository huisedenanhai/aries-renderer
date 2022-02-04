#pragma once

#include "../Pipeline.h"
#include "../RenderGraph.h"
#include "../View.h"

namespace ars::render::vk {
class ToneMapping {
  public:
    explicit ToneMapping(View *view);

    void render(RenderGraph &rg, NamedRT src_rt, NamedRT dst_rt);

  private:
    void execute(CommandBuffer *cmd, NamedRT src_rt_name, NamedRT dst_rt_name);

    void init_pipeline();

    View *_view = nullptr;
    std::unique_ptr<ComputePipeline> _pipeline{};
};
} // namespace ars::render::vk