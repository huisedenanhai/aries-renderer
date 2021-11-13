#pragma once

#include "../Mesh.h"
#include "../Pipeline.h"
#include "../View.h"

namespace ars::render::vk {
struct LightGizmo {
    Handle<Texture> texture{};
    float width = 0.1f;
};

class OverlayRenderer : public IOverlay {
  public:
    explicit OverlayRenderer(View *view);
    ~OverlayRenderer() override;

    glm::vec4 outline_color(uint8_t group) override;
    void set_outline_color(uint8_t group, const glm::vec4 &color) override;
    void set_light_gizmo(const std::shared_ptr<ITexture> &texture,
                         float width) override;
    void draw_outline(uint8_t group,
                      const math::XformTRS<float> &xform,
                      const std::shared_ptr<IMesh> &mesh) override;

    [[nodiscard]] LightGizmo light_gizmo() const;

    bool need_render() const;
    void render(CommandBuffer *cmd, NamedRT dst_rt_name);

  private:
    void init_forward_render_pass();
    void init_billboard_pipeline();
    void ensure_outline_rts();
    void render_outline(CommandBuffer *cmd, const Handle<Texture> &dst_rt);
    void render_billboard(CommandBuffer *cmd, const Handle<Texture> &dst_rt);
    bool need_render_outline() const;
    bool need_render_billboard() const;

    View *_view = nullptr;
    std::array<glm::vec4, 256> _outline_colors{};

    struct OutlineDrawRequest {
        uint8_t group = 0;
        math::XformTRS<float> xform{};
        std::shared_ptr<Mesh> mesh{};
    };

    RenderTargetId _outline_id_rt{};
    RenderTargetId _outline_depth_rt{};
    std::vector<OutlineDrawRequest> _outline_draw_requests{};
    std::unique_ptr<ComputePipeline> _outline_detect_pipeline{};

    LightGizmo _light_gizmo{};

    std::unique_ptr<RenderPass> _overlay_forward_pass;
    std::unique_ptr<GraphicsPipeline> _billboard_pipeline;
};
} // namespace ars::render::vk