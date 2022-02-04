#pragma once

#include "../RenderGraph.h"
#include "../View.h"
#include "../Vulkan.h"
#include <memory>

namespace ars::render::vk {
class OpaqueGeometry;
class DeferredShading;
class GenerateHierarchyZ;
class ScreenSpaceReflection;
class ToneMapping;
class QuerySelection;

class Renderer {
  public:
    explicit Renderer(View *view);
    ~Renderer();

    NamedRT render(RenderGraph &rg);
    std::vector<uint64_t>
    query_selection(uint32_t x, uint32_t y, uint32_t width, uint32_t height);

  private:
    View *_view = nullptr;
    std::unique_ptr<OpaqueGeometry> _opaque_geometry{};
    std::unique_ptr<DeferredShading> _deferred_shading{};
    std::unique_ptr<GenerateHierarchyZ> _generate_hierarchy_z{};
    std::unique_ptr<ScreenSpaceReflection> _screen_space_reflection{};
    std::unique_ptr<ToneMapping> _tone_mapping{};
    std::unique_ptr<QuerySelection> _query_selection{};
};
} // namespace ars::render::vk