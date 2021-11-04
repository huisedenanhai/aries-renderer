#include "Renderer.h"
#include "OpaqueDeferred.h"

namespace ars::render::vk {
NamedRT Renderer::render(CommandBuffer *cmd) {
    _opaque_deferred->render(cmd);
    return NamedRT_FinalColor0;
}

Renderer::Renderer(View *view) : _view(view) {
    _opaque_deferred = std::make_unique<OpaqueDeferred>(_view);
}

Renderer::~Renderer() = default;
} // namespace ars::render::vk