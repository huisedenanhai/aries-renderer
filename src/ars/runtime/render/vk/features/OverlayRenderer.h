#pragma once

#include "../Mesh.h"
#include "../Pipeline.h"
#include "../RenderGraph.h"
#include "../View.h"
#include "Drawer.h"

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
    void draw_outline(uint8_t group, IRenderObject *rd_object) override;
    void draw_line(const glm::vec3 &from,
                   const glm::vec3 &to,
                   const glm::vec4 &color) override;

    [[nodiscard]] LightGizmo light_gizmo() const;

    void render(RenderGraph &rg, NamedRT dst_rt_name);

  private:
    void init_billboard_pipeline();
    void init_line_pipeline();
    void ensure_outline_rts();
    void render_object_ids(CommandBuffer *cmd);
    void calculate_outline(CommandBuffer *cmd, const Handle<Texture> &dst_rt);
    void render_billboard(CommandBuffer *cmd);
    void render_line(CommandBuffer *cmd);
    bool need_render_outline() const;
    bool need_forward_pass() const;
    bool need_render_billboard() const;
    bool need_render_line() const;
    SubpassInfo overlay_pass() const;
    void add_outline_draw_request(uint8_t group, const DrawRequest &req);

    View *_view = nullptr;
    std::array<glm::vec4, 256> _outline_colors{};

    RenderTargetId _outline_id_rt{};
    RenderTargetId _outline_depth_rt{};
    std::vector<DrawRequest> _outline_draw_requests{};
    std::unique_ptr<ComputePipeline> _outline_detect_pipeline{};

    LightGizmo _light_gizmo{};

    std::unique_ptr<GraphicsPipeline> _billboard_pipeline;

    std::vector<glm::vec3> _line_vert_pos{};
    std::vector<glm::vec4> _line_vert_color{};
    std::unique_ptr<GraphicsPipeline> _line_pipeline;
};
} // namespace ars::render::vk