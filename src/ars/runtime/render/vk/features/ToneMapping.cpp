#include "ToneMapping.h"
#include "../Profiler.h"

namespace ars::render::vk {
ToneMapping::ToneMapping(View *view) : _view(view) {
    init_pipeline();
}

void ToneMapping::init_pipeline() {
    _pipeline = ComputePipeline::create(_view->context(), "ToneMapping.comp");
}

void ToneMapping::render(RenderGraph &rg, NamedRT src_rt, NamedRT dst_rt) {
    rg.add_pass(
        [&](RenderGraphPassBuilder &builder) {
            builder.compute_shader_read(src_rt);
            builder.compute_shader_write(dst_rt);
        },
        [=](CommandBuffer *cmd) { execute(cmd, src_rt, dst_rt); });
}

void ToneMapping::execute(CommandBuffer *cmd,
                          NamedRT src_rt_name,
                          NamedRT dst_rt_name) {
    ARS_PROFILER_SAMPLE_VK(cmd, "Tone Mapping", 0xFF81EFA2);

    _pipeline->bind(cmd);

    auto src_rt = _view->render_target(src_rt_name);
    auto dst_rt = _view->render_target(dst_rt_name);
    auto extent = src_rt->info().extent;

    DescriptorEncoder desc{};
    desc.set_texture(0, 0, src_rt.get());
    desc.set_texture(0, 1, dst_rt.get());

    struct Param {
        int width;
        int height;
    };

    Param param{};
    param.width = static_cast<int>(extent.width);
    param.height = static_cast<int>(extent.height);

    desc.set_buffer_data(1, 0, param);
    desc.commit(cmd, _pipeline.get());

    _pipeline->local_size().dispatch(cmd, param.width, param.height, 1);
}
} // namespace ars::render::vk