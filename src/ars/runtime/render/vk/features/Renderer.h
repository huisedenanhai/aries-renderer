#pragma once

#include "OpaqueDeferred.h"
#include <memory>

namespace ars::render::vk {
class View;

class Renderer {
  public:
    explicit Renderer(View *view);
    ~Renderer() = default;

    void render(CommandBuffer *cmd);

  private:
    View *_view = nullptr;
    std::unique_ptr<OpaqueDeferred> _opaque_deferred{};
};
} // namespace ars::render::vk