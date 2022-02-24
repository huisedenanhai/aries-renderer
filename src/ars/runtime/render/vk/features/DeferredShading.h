#pragma once

#include "../Pipeline.h"
#include "../RenderGraph.h"
#include "../View.h"

namespace ars::render::vk {
class DeferredShading {
  public:
    explicit DeferredShading(View *view);

    void render(RenderGraph &rg);

  private:
    void execute(CommandBuffer *cmd);
    void init_render_pass();
    std::unique_ptr<GraphicsPipeline> create_pipeline(const std::string &flag);
    void init_pipeline();
    void set_up_shade(GraphicsPipeline *pipeline, CommandBuffer *cmd);
    void shade_unlit(CommandBuffer *cmd);
    void shade_reflection_emission(CommandBuffer *cmd);
    void shade_directional_light(CommandBuffer *cmd);
    void shade_point_light(CommandBuffer *cmd);
    void shade_sun(CommandBuffer *cmd);

    View *_view = nullptr;
    std::unique_ptr<GraphicsPipeline> _lit_point_light_pipeline{};
    std::unique_ptr<GraphicsPipeline> _lit_directional_light_pipeline{};
    std::unique_ptr<GraphicsPipeline> _lit_reflection_pipeline{};
    std::unique_ptr<GraphicsPipeline> _lit_sun_pipeline{};
    std::unique_ptr<GraphicsPipeline> _unlit_pipeline{};

    std::unique_ptr<RenderPass> _render_pass{};
};
} // namespace ars::render::vk