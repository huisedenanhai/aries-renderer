#pragma once

#include "../Mesh.h"
#include "../View.h"

namespace ars::render::vk {
class OverlayRenderer : public IOverlay {
  public:
    explicit OverlayRenderer(View *view);

    glm::vec4 outline_color(uint8_t group) override;
    void set_outline_color(uint8_t group, const glm::vec4 &color) override;
    void draw_outline(uint8_t group,
                      const math::XformTRS<float> &xform,
                      const std::shared_ptr<IMesh> &mesh) override;

    bool need_render() const;
    void render(CommandBuffer *cmd, NamedRT dst_rt);

  private:
    View *_view = nullptr;
    std::array<glm::vec4, 256> _outline_colors{};

    struct OutlineDrawRequest {
        uint8_t group = 0;
        math::XformTRS<float> xform{};
        std::shared_ptr<Mesh> mesh{};
    };
    std::vector<OutlineDrawRequest> _outline_draw_requests{};
};
} // namespace ars::render::vk