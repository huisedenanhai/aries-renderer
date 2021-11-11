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

    void draw_ids(CommandBuffer *cmd,
                  const Handle<Texture> &id_color,
                  const Handle<Texture> &depth,
                  uint32_t count,
                  const glm::mat4 *mvp_arr,
                  const uint32_t *ids,
                  const std::shared_ptr<Mesh> *meshes) const;

  private:
    void init_render_pass();
    void init_pipeline();

    View *_view = nullptr;
    std::unique_ptr<RenderPass> _render_pass{};
    std::unique_ptr<GraphicsPipeline> _pipeline{};
};
} // namespace ars::render::vk