#include "Renderer.h"
#include "OpaqueDeferred.h"
#include "ToneMapping.h"

namespace ars::render::vk {
NamedRT Renderer::render(CommandBuffer *cmd) {
    _opaque_deferred->render(cmd);
    barrier(cmd,
            _opaque_deferred->dst_dependencies(),
            _tone_mapping->src_dependencies());
    _tone_mapping->render(cmd);
    return NamedRT_FinalColor1;
}

Renderer::Renderer(View *view) : _view(view) {
    _opaque_deferred =
        std::make_unique<OpaqueDeferred>(_view, NamedRT_FinalColor0);
    _tone_mapping = std::make_unique<ToneMapping>(
        _view, NamedRT_FinalColor0, NamedRT_FinalColor1);
}

Renderer::~Renderer() = default;
} // namespace ars::render::vk