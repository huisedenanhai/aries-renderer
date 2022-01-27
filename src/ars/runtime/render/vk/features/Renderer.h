#pragma once

#include "../RenderGraph.h"
#include "../View.h"
#include "../Vulkan.h"
#include <memory>

namespace ars::render::vk {
class QuerySelection;

class Renderer {
  public:
    explicit Renderer(View *view);
    ~Renderer();

    NamedRT render(CommandBuffer *cmd);
    std::vector<uint64_t>
    query_selection(uint32_t x, uint32_t y, uint32_t width, uint32_t height);

  private:
    template <typename Pass, typename... Args> void add_pass(Args &&...args) {
        _passes.template emplace_back(
            std::make_unique<Pass>(std::forward<Args>(args)...));
    }

    View *_view = nullptr;
    std::vector<std::unique_ptr<IRenderGraphPass>> _passes{};
    std::unique_ptr<QuerySelection> _query_selection{};
};
} // namespace ars::render::vk