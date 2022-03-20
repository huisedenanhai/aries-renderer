#pragma once

#include "../Mesh.h"
#include "../Pipeline.h"
#include "../RenderPass.h"
#include "../Texture.h"
#include "../Vulkan.h"
#include "Renderer.h"

namespace ars::render::vk {
constexpr VkFormat ID_COLOR_ATTACHMENT_FORMAT = VK_FORMAT_R32_UINT;
constexpr VkFormat ID_DEPTH_STENCIL_ATTACHMENT_FORMAT = VK_FORMAT_D32_SFLOAT;

class View;
class Material;

struct DrawRequest {
    glm::mat4 M;
    Mesh *mesh;
    Material *material;
};

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

    void draw(CommandBuffer *cmd,
              const glm::mat4 &P,
              const glm::mat4 &V,
              RenderPassID pass_id,
              uint32_t count,
              const DrawRequest *requests);

    void draw(CommandBuffer *cmd,
              View *view,
              RenderPassID pass_id,
              uint32_t count,
              const DrawRequest *requests);

    // Use the view holding the drawer
    void draw(CommandBuffer *cmd,
              RenderPassID pass_id,
              uint32_t count,
              const DrawRequest *requests);

  private:
    void init_render_pass();
    void init_draw_id_pipeline();
    void init_draw_id_billboard_alpha_clip();

    void draw(CommandBuffer *cmd,
              const glm::mat4 &P,
              const glm::mat4 &V,
              const Handle<Buffer> &view_transform_buffer,
              RenderPassID pass_id,
              uint32_t count,
              const DrawRequest *requests);

    View *_view = nullptr;
    std::unique_ptr<RenderPass> _render_pass{};
    std::unique_ptr<GraphicsPipeline> _draw_id_pipeline{};
    std::unique_ptr<GraphicsPipeline> _draw_id_billboard_alpha_clip_pipeline{};
};
} // namespace ars::render::vk