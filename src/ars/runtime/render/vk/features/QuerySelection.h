#pragma once

#include "../Pipeline.h"
#include "../RenderPass.h"
#include <vector>

namespace ars::render::vk {
class View;

class QuerySelection {
  public:
    explicit QuerySelection(View *view);

    std::vector<uint64_t>
    query_selection(uint32_t x, uint32_t y, uint32_t width, uint32_t height);

  private:
    void init_render_pass();
    void init_pipeline();

    View *_view = nullptr;
    std::unique_ptr<RenderPass> _render_pass{};
    std::unique_ptr<GraphicsPipeline> _pipeline{};
};
} // namespace ars::render::vk