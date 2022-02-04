#include "RenderGraph.h"
#include "Context.h"
#include "Profiler.h"

namespace ars::render::vk {
void PassDependency::barrier(CommandBuffer *cmd,
                             const std::vector<PassDependency> &src_deps,
                             const std::vector<PassDependency> &dst_deps) {
    std::vector<VkImageMemoryBarrier> barriers{};
    barriers.reserve(src_deps.size() + dst_deps.size());

    VkPipelineStageFlags src_stage_mask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    VkPipelineStageFlags dst_stage_mask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;

    for (auto &dep : src_deps) {
        assert(dep.texture.get() != nullptr);

        src_stage_mask |= dep.stage_mask;

        VkImageMemoryBarrier b{VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
        b.subresourceRange = dep.texture->subresource_range();
        b.srcAccessMask = dep.access_mask;
        b.dstAccessMask = 0;

        // Maybe we should ignore dep.layout for src?
        assert(dep.layout == dep.texture->layout());

        // If there is only src dependency, do not change image layout
        b.oldLayout = dep.layout;
        b.newLayout = dep.layout;
        b.image = dep.texture->image();

        barriers.push_back(b);
    }

    auto find_src_barrier =
        [&](const PassDependency &dep) -> VkImageMemoryBarrier * {
        assert(dep.texture.get() != nullptr);

        for (auto &b : barriers) {
            if (b.image == dep.texture->image()) {
                return &b;
            }
        }
        return nullptr;
    };

    for (auto &dep : dst_deps) {
        dst_stage_mask |= dep.stage_mask;

        auto src_barrier = find_src_barrier(dep);
        if (src_barrier != nullptr) {
            auto &b = *src_barrier;
            b.dstAccessMask = dep.access_mask;
            b.newLayout = dep.layout;
        } else {
            VkImageMemoryBarrier b{VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
            b.subresourceRange = dep.texture->subresource_range();
            b.srcAccessMask = 0;
            b.dstAccessMask = dep.access_mask;
            b.oldLayout = dep.texture->layout();
            b.newLayout = dep.layout;
            b.image = dep.texture->image();

            barriers.push_back(b);
        }
    }

    cmd->PipelineBarrier(src_stage_mask,
                         dst_stage_mask,
                         0,
                         0,
                         nullptr,
                         0,
                         nullptr,
                         static_cast<uint32_t>(barriers.size()),
                         barriers.data());

    for (auto &dep : dst_deps) {
        dep.texture->assure_layout(dep.layout);
    }
}

void RenderGraph::execute() {
    _view->context()->queue()->submit_once([&](CommandBuffer *cmd) {
        ARS_PROFILER_SAMPLE_VK(cmd, "Render Graph Execute", 0xFF772183);
        for (int i = 0; i < _passes.size(); i++) {
            if (i > 0) {
                PassDependency::barrier(cmd,
                                        _passes[i - 1]->dst_dependencies(),
                                        _passes[i]->src_dependencies());
            }
            _passes[i]->execute(cmd);
        }
    });
}

void RenderGraph::compile() {
    ARS_PROFILER_SAMPLE("Render Graph Compile", 0xFF125512);
    // For now, do nothing
}

void RenderGraph::add_pass_internal(std::unique_ptr<IRenderGraphPass> pass) {
    _passes.emplace_back(std::move(pass));
}

RenderGraph::RenderGraph(View *view) : _view(view) {}

std::vector<PassDependency> RenderGraphPass::src_dependencies() {
    return _src_dependencies;
}

std::vector<PassDependency> RenderGraphPass::dst_dependencies() {
    return _dst_dependencies;
}

void RenderGraphPass::execute(CommandBuffer *cmd) {
    if (_execute_action) {
        _execute_action(cmd);
    }
}

RenderGraphPass::RenderGraphPass(std::function<void(CommandBuffer *)> action)
    : _execute_action(std::move(action)) {}

RenderGraphPassBuilder::RenderGraphPassBuilder(View *view,
                                               RenderGraphPass *pass)
    : _view(view), _pass(pass),
      _src_deps_builder(view, &pass->_src_dependencies),
      _dst_deps_builder(view, &pass->_dst_dependencies) {}

PassDependencyBuilder &RenderGraphPassBuilder::src_dependencies() {
    return _src_deps_builder;
}

PassDependencyBuilder &RenderGraphPassBuilder::dst_dependencies() {
    return _dst_deps_builder;
}

PassDependencyBuilder::PassDependencyBuilder(View *view,
                                             std::vector<PassDependency> *deps)
    : _view(view), _deps(deps) {
    assert(_view != nullptr);
    assert(_deps != nullptr);
}

void PassDependencyBuilder::add(NamedRT rt,
                                VkAccessFlags access_mask,
                                VkPipelineStageFlags stage_mask,
                                VkImageLayout layout) {
    add(_view->render_target(rt), access_mask, stage_mask, layout);
}

void PassDependencyBuilder::add(const Handle<Texture> &rt,
                                VkAccessFlags access_mask,
                                VkPipelineStageFlags stage_mask,
                                VkImageLayout layout) {
    PassDependency dep{};
    dep.texture = rt;
    dep.access_mask = access_mask;
    dep.stage_mask = stage_mask;
    dep.layout = layout;

    _deps->emplace_back(std::move(dep));
}
} // namespace ars::render::vk
