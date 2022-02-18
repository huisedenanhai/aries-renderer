#include "Renderer.h"
#include "../Effect.h"
#include "../Profiler.h"
#include "../Sky.h"
#include "DeferredShading.h"
#include "OpaqueGeometry.h"
#include "QuerySelection.h"
#include "ScreenSpaceReflection.h"
#include "ToneMapping.h"

namespace ars::render::vk {
NamedRT Renderer::render(RenderGraph &rg) {
    ARS_PROFILER_SAMPLE("Build Render Graph", 0xFF772641);

    _view->effect_vk()->background_vk()->sky_vk()->render(rg);
    _opaque_geometry->render(rg);
    _generate_hierarchy_z->render(rg);
    _screen_space_reflection->render(rg);
    _deferred_shading->render(rg, NamedRT_LinearColor);

    Texture::generate_mipmap(_view->render_target(NamedRT_LinearColor), rg);

    // Post-processing
    NamedRT pp_buffer[2] = {NamedRT_PostProcessing0, NamedRT_PostProcessing1};
    uint32_t pp_target_index = 0;
    _tone_mapping->render(rg, NamedRT_LinearColor, NamedRT_PostProcessing0);

    return NamedRT_PostProcessing0;
}

Renderer::Renderer(View *view) : _view(view) {
    _opaque_geometry = std::make_unique<OpaqueGeometry>(_view);
    _deferred_shading = std::make_unique<DeferredShading>(_view);
    _generate_hierarchy_z = std::make_unique<GenerateHierarchyZ>(_view);
    _screen_space_reflection = std::make_unique<ScreenSpaceReflection>(_view);
    _tone_mapping = std::make_unique<ToneMapping>(_view);

    _query_selection = std::make_unique<QuerySelection>(_view);

    _add_inplace_pipeline =
        ComputePipeline::create(_view->context(), "AddInplace.comp");
}

std::vector<uint64_t> Renderer::query_selection(uint32_t x,
                                                uint32_t y,
                                                uint32_t width,
                                                uint32_t height) {
    return _query_selection->query_selection(x, y, width, height);
}

void Renderer::add_inplace(RenderGraph &rg,
                           NamedRT add_src_rt,
                           NamedRT target_rt) {
    rg.add_pass(
        [&](RenderGraphPassBuilder &builder) {
            builder.compute_shader_read(add_src_rt);
            builder.compute_shader_write(target_rt);
        },
        [=](CommandBuffer *cmd) {
            auto target = _view->render_target(target_rt);
            auto add_src = _view->render_target(add_src_rt);
            auto dst_extent = target->info().extent;

            _add_inplace_pipeline->bind(cmd);
            DescriptorEncoder desc{};
            desc.set_texture(0, 0, target.get());
            desc.set_texture(0, 1, add_src.get());
            desc.commit(cmd, _add_inplace_pipeline.get());
            _add_inplace_pipeline->local_size().dispatch(cmd, dst_extent);
        });
}

Renderer::~Renderer() = default;
} // namespace ars::render::vk