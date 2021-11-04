#pragma once

#include "../Pipeline.h"
#include "../RenderGraph.h"
#include "../RenderPass.h"
#include "../View.h"
#include "../Vulkan.h"
#include <array>

namespace ars::render::vk {
class OpaqueDeferred {
  public:
    explicit OpaqueDeferred(View *view);
    ~OpaqueDeferred() = default;

    void render(CommandBuffer *cmd);

    [[nodiscard]] std::vector<PassDependency> dst_dependencies();

  private:
    void init_render_pass();
    void init_geometry_pass_pipeline();
    void init_shading_pass_pipeline();
    [[nodiscard]] std::array<NamedRT, 5> geometry_pass_rt_names() const;
    [[nodiscard]] std::array<Handle<Texture>, 5> geometry_pass_rts() const;

    void geometry_pass(CommandBuffer *cmd);
    void geometry_pass_barrier(const CommandBuffer *cmd) const;
    void shading_pass(CommandBuffer *cmd) const;

    View *_view = nullptr;
    std::unique_ptr<RenderPass> _render_pass{};
    std::unique_ptr<GraphicsPipeline> _geometry_pass_pipeline{};
    std::unique_ptr<ComputePipeline> _shading_pass_pipeline{};
};
} // namespace ars::render::vk