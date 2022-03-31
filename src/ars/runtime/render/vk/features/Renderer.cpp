#include "Renderer.h"
#include "../Effect.h"
#include "../Profiler.h"
#include "../Scene.h"
#include "../Sky.h"
#include "DeferredShading.h"
#include "OpaqueGeometry.h"
#include "QuerySelection.h"
#include "ScreenSpaceReflection.h"
#include "ToneMapping.h"

namespace ars::render::vk {
NamedRT Renderer::render(RenderGraph &rg, const RenderOptions &options) {
    ARS_PROFILER_SAMPLE("Build Render Graph", 0xFF772641);

    auto w_div_h = _view->size().w_div_h();
    auto frustum = transform_frustum(_view->xform().matrix_no_scale(),
                                     _view->camera().frustum(w_div_h));

    // Use custom culling option
    if (options.culling.has_value()) {
        frustum = transform_frustum(
            options.culling->culling_camera_xform.matrix_no_scale(),
            options.culling->culling_camera_data.frustum(w_div_h));
    }

    auto culling_result = _view->scene_vk()->cull(frustum);

    auto sky = _view->effect_vk()->background_vk()->sky_vk();

    sky->update(_view, rg);
    _opaque_geometry->render(rg, culling_result);
    _generate_hierarchy_z->render(rg);
    _screen_space_reflection->render(rg);
    _deferred_shading->render(rg);
    sky->render_background(_view, rg);

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

SubpassInfo RendererContextData::subpass(RenderPassID pass_id) const {
    auto id = static_cast<uint32_t>(pass_id);
    assert(id < _subpasses.size());
    return _subpasses[id];
}

RendererContextData::RendererContextData(Context *context) : _context(context) {
    init_render_passes();
}

void RendererContextData::init_geometry_pass() {
    RenderPassAttachmentInfo colors[4]{};
    colors[0].format = RT_FORMAT_GBUFFER0;
    colors[1].format = RT_FORMAT_GBUFFER1;
    colors[2].format = RT_FORMAT_GBUFFER2;
    colors[3].format = RT_FORMAT_GBUFFER3;

    RenderPassAttachmentInfo depth{};
    depth.format = RT_FORMAT_DEPTH;

    _geometry_pass =
        RenderPass::create_with_single_pass(_context, 4, colors, &depth);
}

void RendererContextData::init_shading_pass() {
    RenderPassAttachmentInfo color{};
    color.format = RT_FORMAT_DEFAULT_HDR;

    _shading_pass =
        RenderPass::create_with_single_pass(_context, 1, &color, nullptr);
}

void RendererContextData::init_overlay_pass() {
    RenderPassAttachmentInfo color{};
    color.format = RT_FORMAT_DEFAULT_HDR;
    color.samples = VK_SAMPLE_COUNT_1_BIT;
    color.initial_layout = VK_IMAGE_LAYOUT_GENERAL;
    color.final_layout = VK_IMAGE_LAYOUT_GENERAL;
    color.load_op = VK_ATTACHMENT_LOAD_OP_LOAD;
    color.store_op = VK_ATTACHMENT_STORE_OP_STORE;

    RenderPassAttachmentInfo depth{};
    depth.format = RT_FORMAT_DEPTH;
    depth.samples = VK_SAMPLE_COUNT_1_BIT;
    depth.initial_layout = VK_IMAGE_LAYOUT_GENERAL;
    depth.final_layout = VK_IMAGE_LAYOUT_GENERAL;
    depth.load_op = VK_ATTACHMENT_LOAD_OP_LOAD;
    depth.store_op = VK_ATTACHMENT_STORE_OP_STORE;

    _overlay_pass =
        RenderPass::create_with_single_pass(_context, 1, &color, &depth);
}

void RendererContextData::init_render_passes() {
    init_geometry_pass();
    init_shading_pass();
    init_overlay_pass();

    _subpasses.resize(RenderPassID_Count);

    _subpasses[RenderPassID_Geometry] = {_geometry_pass.get(), 0};
    _subpasses[RenderPassID_Shading] = {_shading_pass.get(), 0};
    _subpasses[RenderPassID_Overlay] = {_overlay_pass.get(), 0};
}
} // namespace ars::render::vk