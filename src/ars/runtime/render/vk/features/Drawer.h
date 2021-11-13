#pragma once

#include "../Mesh.h"
#include "../Pipeline.h"
#include "../RenderPass.h"
#include "../Texture.h"
#include "../Vulkan.h"

namespace ars::render::vk {
constexpr VkFormat ID_COLOR_ATTACHMENT_FORMAT = VK_FORMAT_R32_UINT;
constexpr VkFormat ID_DEPTH_STENCIL_ATTACHMENT_FORMAT = VK_FORMAT_D32_SFLOAT;

class View;

class Drawer {
  public:
    explicit Drawer(View *view);

    RenderPass *draw_id_render_pass() const;

    void draw_ids(CommandBuffer *cmd,
                  uint32_t count,
                  const glm::mat4 *mvp_arr,
                  const uint32_t *ids,
                  const std::shared_ptr<Mesh> *meshes) const;

    void draw_ids_billboard_alpha_clip(CommandBuffer *cmd,
                                       uint32_t count,
                                       const glm::mat4 *mvp_arr,
                                       const uint32_t *ids,
                                       const Handle<Texture> *textures) const;

  private:
    void init_render_pass();
    void init_draw_id_pipeline();
    void init_draw_id_billboard_alpha_clip();

    View *_view = nullptr;
    std::unique_ptr<RenderPass> _render_pass{};
    std::unique_ptr<GraphicsPipeline> _draw_id_pipeline{};
    std::unique_ptr<GraphicsPipeline> _draw_id_billboard_alpha_clip_pipeline{};
};
} // namespace ars::render::vk