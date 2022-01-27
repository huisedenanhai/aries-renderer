#include "Renderer.h"
#include "DeferredShading.h"
#include "OpaqueGeometry.h"
#include "QuerySelection.h"
#include "ScreenSpaceReflection.h"
#include "ToneMapping.h"

namespace ars::render::vk {
NamedRT Renderer::render(CommandBuffer *cmd) {
    for (int i = 0; i < _passes.size(); i++) {
        if (i > 0) {
            PassDependency::barrier(cmd,
                                    _passes[i - 1]->dst_dependencies(),
                                    _passes[i]->src_dependencies());
        }
        _passes[i]->render(cmd);
    }
    return NamedRT_FinalColor1;
}

Renderer::Renderer(View *view) : _view(view) {
    add_pass<OpaqueGeometry>(_view);
    add_pass<DeferredShading>(_view, NamedRT_FinalColor0);
    add_pass<GenerateHierarchyZ>(_view);
    add_pass<ScreenSpaceReflection>(_view);
    add_pass<ToneMapping>(_view, NamedRT_FinalColor0, NamedRT_FinalColor1);

    _query_selection = std::make_unique<QuerySelection>(_view);
}

std::vector<uint64_t> Renderer::query_selection(uint32_t x,
                                                uint32_t y,
                                                uint32_t width,
                                                uint32_t height) {
    return _query_selection->query_selection(x, y, width, height);
}

Renderer::~Renderer() = default;
} // namespace ars::render::vk