#pragma once

#include "../View.h"
#include "../Vulkan.h"
#include <memory>

namespace ars::render::vk {
class OpaqueGeometry;
class DeferredShading;
class ToneMapping;

class Renderer {
  public:
    explicit Renderer(View *view);
    ~Renderer();

    NamedRT render(CommandBuffer *cmd);

  private:
    View *_view = nullptr;
    std::unique_ptr<OpaqueGeometry> _opaque_geometry{};
    std::unique_ptr<DeferredShading> _deferred_shading{};
    std::unique_ptr<ToneMapping> _tone_mapping{};
};
} // namespace ars::render::vk