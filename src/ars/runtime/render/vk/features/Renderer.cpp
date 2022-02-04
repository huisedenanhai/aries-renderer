#include "Renderer.h"
#include "../Profiler.h"
#include "DeferredShading.h"
#include "OpaqueGeometry.h"
#include "QuerySelection.h"
#include "ScreenSpaceReflection.h"
#include "ToneMapping.h"

namespace ars::render::vk {
NamedRT Renderer::render(RenderGraph &rg) {
    ARS_PROFILER_SAMPLE("Build Render Graph", 0xFF772641);
    NamedRT final_colors[2] = {NamedRT_FinalColor0, NamedRT_FinalColor1};
    auto flip_pingpong_buffer = [&]() {
        std::swap(final_colors[0], final_colors[1]);
    };

    _opaque_geometry->render(rg);
    flip_pingpong_buffer();
    _deferred_shading->render(rg, final_colors[0]);
    _generate_hierarchy_z->render(rg);
    flip_pingpong_buffer();
    _screen_space_reflection->render(rg, final_colors[1], final_colors[0]);
    flip_pingpong_buffer();
    _tone_mapping->render(rg, final_colors[1], final_colors[0]);

    return final_colors[0];
}

Renderer::Renderer(View *view) : _view(view) {
    _opaque_geometry = std::make_unique<OpaqueGeometry>(_view);
    _deferred_shading = std::make_unique<DeferredShading>(_view);
    _generate_hierarchy_z = std::make_unique<GenerateHierarchyZ>(_view);
    _screen_space_reflection = std::make_unique<ScreenSpaceReflection>(_view);
    _tone_mapping = std::make_unique<ToneMapping>(_view);

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