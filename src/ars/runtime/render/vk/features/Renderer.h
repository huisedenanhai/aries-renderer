#pragma once

#include "../View.h"
#include "../Vulkan.h"
#include <memory>

namespace ars::render::vk {
class OpaqueDeferred;
class ToneMapping;

class Renderer {
  public:
    explicit Renderer(View *view);
    ~Renderer();

    NamedRT render(CommandBuffer *cmd);

  private:
    View *_view = nullptr;
    std::unique_ptr<OpaqueDeferred> _opaque_deferred{};
    std::unique_ptr<ToneMapping> _tone_mapping{};
};
} // namespace ars::render::vk