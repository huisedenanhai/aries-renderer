#include "Renderer.h"
#include "DeferredShading.h"
#include "OpaqueGeometry.h"
#include "ToneMapping.h"

namespace ars::render::vk {
NamedRT Renderer::render(CommandBuffer *cmd) {
    _opaque_geometry->render(cmd);
    barrier(cmd,
            _opaque_geometry->dst_dependencies(),
            _deferred_shading->src_dependencies());
    _deferred_shading->render(cmd);
    barrier(cmd,
            _deferred_shading->dst_dependencies(),
            _tone_mapping->src_dependencies());
    _tone_mapping->render(cmd);
    return NamedRT_FinalColor1;
}

Renderer::Renderer(View *view) : _view(view) {
    _opaque_geometry = std::make_unique<OpaqueGeometry>(_view);
    _deferred_shading =
        std::make_unique<DeferredShading>(_view, NamedRT_FinalColor0);
    _tone_mapping = std::make_unique<ToneMapping>(
        _view, NamedRT_FinalColor0, NamedRT_FinalColor1);
}

Renderer::~Renderer() = default;
} // namespace ars::render::vk