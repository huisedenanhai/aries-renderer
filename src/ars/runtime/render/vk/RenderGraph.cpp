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
        RenderGraphPass *last_pass = nullptr;
        for (auto &pass_info : _passes) {
            if (!pass_info.active) {
                continue;
            }
            auto cur_pass = &pass_info.pass;
            // TODO optimize barrier
            if (last_pass) {
                PassDependency::barrier(cmd,
                                        last_pass->dst_dependencies(),
                                        cur_pass->src_dependencies());
            } else {
                // Perform layout transformation for those uninitialized write
                // deps
                PassDependency::barrier(cmd, {}, cur_pass->src_dependencies());
            }
            cur_pass->execute(cmd);
            last_pass = cur_pass;
        }
    });
}

void RenderGraph::compile() {
    ARS_PROFILER_SAMPLE("Render Graph Compile", 0xFF125512);

    // Culling
    std::set<Texture *> alive{};
    auto rt_manager = _view->rt_manager();
    for (auto rt : _output_rts) {
        alive.insert(rt_manager->get(rt).get());
    }

    for (int i = static_cast<int>(_passes.size()) - 1; i >= 0; i--) {
        auto &info = _passes[i];
        auto &pass = info.pass;

        info.active = false;
        if (pass.has_side_effect) {
            info.active = true;
        }

        // Kill all full rewrites
        for (auto &d : pass.dependencies) {
            if (!(d.access_mask & VULKAN_ACCESS_WRITE_MASK)) {
                continue;
            }
            auto it = alive.find(d.texture.get());
            if (it != alive.end()) {
                info.active = true;
                if (d.full_rewrite) {
                    alive.erase(it);
                }
            }
        }
        if (info.active) {
            // Gen all reads
            for (auto &d : pass.dependencies) {
                if (!(d.access_mask & VULKAN_ACCESS_READ_MASK)) {
                    continue;
                }
                alive.insert(d.texture.get());
            }
        }
    }
    // TODO virtual resource management, automatic scheduling
}

RenderGraph::RenderGraph(View *view) : _view(view) {}

RenderGraphPass *RenderGraph::alloc_pass() {
    _passes.emplace_back();
    return &_passes.back().pass;
}

void RenderGraph::output(RenderTargetId id) {
    _output_rts.insert(id);
}

void RenderGraph::output(NamedRT rt) {
    output(_view->rt_id(rt));
}

View *RenderGraph::view() const {
    return _view;
}

std::vector<PassDependency> RenderGraphPass::src_dependencies() const {
    return dependencies;
}

std::vector<PassDependency> RenderGraphPass::dst_dependencies() const {
    std::vector<PassDependency> deps{};
    deps.reserve(dependencies.size());
    for (auto &d : deps) {
        if (d.access_mask & VULKAN_ACCESS_WRITE_MASK) {
            deps.push_back(d);
        }
    }
    return deps;
}

void RenderGraphPass::execute(CommandBuffer *cmd) const {
    if (execute_action) {
        execute_action(cmd);
    }
}

RenderGraphPassBuilder::RenderGraphPassBuilder(View *view,
                                               RenderGraphPass *pass)
    : _view(view), _pass(pass), _deps_builder(view, &pass->dependencies) {}

void RenderGraphPassBuilder::has_side_effect(bool side_effect) {
    _pass->has_side_effect = side_effect;
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
                                bool full_rewrite,
                                VkImageLayout layout) {
    add(_view->render_target(rt),
        access_mask,
        stage_mask,
        full_rewrite,
        layout);
}

void PassDependencyBuilder::add(const Handle<Texture> &rt,
                                VkAccessFlags access_mask,
                                VkPipelineStageFlags stage_mask,
                                bool full_rewrite,
                                VkImageLayout layout) {
    PassDependency dep{};
    dep.texture = rt;
    dep.access_mask = access_mask;
    dep.stage_mask = stage_mask;
    dep.layout = layout;
    dep.full_rewrite = full_rewrite;

    _deps->emplace_back(std::move(dep));
}

void PassDependencyBuilder::add(RenderTargetId rt,
                                VkAccessFlags access_mask,
                                VkPipelineStageFlags stage_mask,
                                bool full_rewrite,
                                VkImageLayout layout) {
    add(_view->rt_manager()->get(rt),
        access_mask,
        stage_mask,
        full_rewrite,
        layout);
}
} // namespace ars::render::vk
