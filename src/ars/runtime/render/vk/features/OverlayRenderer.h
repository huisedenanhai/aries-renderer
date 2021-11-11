#pragma once

#include "../Mesh.h"
#include "../Pipeline.h"
#include "../View.h"

namespace ars::render::vk {
class OverlayRenderer : public IOverlay {
  public:
    explicit OverlayRenderer(View *view);
    ~OverlayRenderer() override;

    glm::vec4 outline_color(uint8_t group) override;
    void set_outline_color(uint8_t group, const glm::vec4 &color) override;
    void draw_outline(uint8_t group,
                      const math::XformTRS<float> &xform,
                      const std::shared_ptr<IMesh> &mesh) override;

    bool need_render() const;
    void render(CommandBuffer *cmd, NamedRT dst_rt);

  private:
    void ensure_rts();

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
};
} // namespace ars::render::vk