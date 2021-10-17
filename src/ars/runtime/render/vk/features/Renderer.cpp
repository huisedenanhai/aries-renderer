#include "Renderer.h"

namespace ars::render::vk {
void Renderer::render(CommandBuffer *cmd) {
    _opaque_deferred->render(cmd);
}

Renderer::Renderer(View *view) : _view(view) {
    _opaque_deferred = std::make_unique<OpaqueDeferred>(_view);
}
} // namespace ars::render::vk