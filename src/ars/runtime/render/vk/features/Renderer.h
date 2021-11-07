#pragma once

#include "../View.h"
#include "../Vulkan.h"
#include <memory>

namespace ars::render::vk {
class OpaqueGeometry;
class DeferredShading;
class ToneMapping;
class QuerySelection;

class Renderer {
  public:
    explicit Renderer(View *view);
    ~Renderer();

    NamedRT render(CommandBuffer *cmd);
    std::vector<uint64_t>
    query_selection(uint32_t x, uint32_t y, uint32_t width, uint32_t height);

  private:
    View *_view = nullptr;
    std::unique_ptr<OpaqueGeometry> _opaque_geometry{};
    std::unique_ptr<DeferredShading> _deferred_shading{};
    std::unique_ptr<ToneMapping> _tone_mapping{};
    std::unique_ptr<QuerySelection> _query_selection{};
};
} // namespace ars::render::vk