#include "ToneMapping.h"

namespace ars::render::vk {
ToneMapping::ToneMapping(View *view, NamedRT src_rt, NamedRT dst_rt)
    : _view(view), _src_rt_name(src_rt), _dst_rt_name(dst_rt) {
    init_pipeline();
}

void ToneMapping::init_pipeline() {
    auto ctx = _view->context();
    auto shader = std::make_unique<Shader>(ctx, "ToneMapping.comp");
    ComputePipelineInfo info{};
    info.shader = shader.get();
    _pipeline = std::make_unique<ComputePipeline>(ctx, info);
}

std::vector<PassDependency> ToneMapping::src_dependencies() {
    std::vector<PassDependency> deps(2);
    {
        auto &d = deps[0];
        d.texture = _view->render_target(_src_rt_name);
        d.access_mask = VK_ACCESS_SHADER_READ_BIT;
        d.stage_mask = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
        d.layout = VK_IMAGE_LAYOUT_GENERAL;
    }
    {
        auto &d = deps[1];
        d.texture = _view->render_target(_dst_rt_name);
        d.access_mask = VK_ACCESS_SHADER_WRITE_BIT;
        d.stage_mask = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
        d.layout = VK_IMAGE_LAYOUT_GENERAL;
    }

    return deps;
}

void ToneMapping::render(CommandBuffer *cmd) {
    _pipeline->bind(cmd);

    auto src_rt = _view->render_target(_src_rt_name);
    auto dst_rt = _view->render_target(_dst_rt_name);
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