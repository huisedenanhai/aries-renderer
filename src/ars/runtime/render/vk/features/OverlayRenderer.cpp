#include "OverlayRenderer.h"

namespace ars::render::vk {
OverlayRenderer::OverlayRenderer(View *view) : _view(view) {
    std::fill(_outline_colors.begin(),
              _outline_colors.end(),
              glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
}

glm::vec4 OverlayRenderer::outline_color(uint8_t group) {
    return _outline_colors[group];
}

void OverlayRenderer::set_outline_color(uint8_t group, const glm::vec4 &color) {
    _outline_colors[group] = color;
}

void OverlayRenderer::draw_outline(uint8_t group,
                                   const math::XformTRS<float> &xform,
                                   const std::shared_ptr<IMesh> &mesh) {
    OutlineDrawRequest request{};
    request.group = group;
    request.xform = xform;
    request.mesh = upcast(mesh);
    _outline_draw_requests.emplace_back(std::move(request));
}

bool OverlayRenderer::need_render() const {
    return !_outline_draw_requests.empty();
}

void OverlayRenderer::render(CommandBuffer *cmd, NamedRT dst_rt) {
    _outline_draw_requests.clear();
}
} // namespace ars::render::vk