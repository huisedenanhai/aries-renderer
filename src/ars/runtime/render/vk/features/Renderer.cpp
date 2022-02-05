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
    _screen_space_reflection->trace_rays(rg);
    _screen_space_reflection->resolve_reflection(rg);
    add_inplace(rg, NamedRT_Reflection, final_colors[0]);

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
            builder.read(add_src_rt,
                         VK_ACCESS_SHADER_READ_BIT,
                         VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
            builder.write(target_rt,
                          VK_ACCESS_SHADER_WRITE_BIT,
                          VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
        },
        [=](CommandBuffer *cmd) {
            auto target = _view->render_target(target_rt);
            auto add_src = _view->render_target(add_src_rt);
            auto dst_extent = target->info().extent;

            _add_inplace_pipeline->bind(cmd);
            DescriptorEncoder desc{};
            desc.set_texture(0, 0, target.get());
            desc.set_texture(0, 1, add_src.get());

            struct Param {
                int32_t width;
                int32_t height;
            };

            Param param{};
            param.width = static_cast<int32_t>(dst_extent.width);
            param.height = static_cast<int32_t>(dst_extent.height);

            desc.set_buffer_data(1, 0, param);
            desc.commit(cmd, _add_inplace_pipeline.get());
            _add_inplace_pipeline->local_size().dispatch(cmd, dst_extent);
        });
}

Renderer::~Renderer() = default;
} // namespace ars::render::vk