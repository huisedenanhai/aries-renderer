#pragma once

#include "../Material.h"
#include "../Mesh.h"
#include "../Pipeline.h"
#include "../RenderPass.h"
#include "../Texture.h"
#include "../Vulkan.h"
#include "Renderer.h"
#include <ars/runtime/core/misc/Span.h>

namespace ars::render::vk {

class View;

struct DrawRequest {
    glm::mat4 M{};
    uint32_t custom_id = 0;
    Mesh *mesh = nullptr;
    MaterialPass material{};
    Skin *skeleton = nullptr;
};

struct InstanceDrawParam {
    glm::mat4 MV;
    glm::mat4 I_MV;
    uint32_t material_id;
    uint32_t instance_id;
    uint32_t custom_id;
    ARS_PADDING_FIELD(uint32_t);
};

struct DrawCallbacks {
    std::function<void(CommandBuffer *)> on_pipeline_bound{};
};

class Drawer {
  public:
    explicit Drawer(View *view);

    RenderPass *draw_id_render_pass() const;

    void draw_ids_billboard_alpha_clip(CommandBuffer *cmd,
                                       uint32_t count,
                                       const glm::mat4 *mvp_arr,
                                       const uint32_t *ids,
                                       const Handle<Texture> *textures) const;

    void draw(CommandBuffer *cmd,
              const glm::mat4 &P,
              const glm::mat4 &V,
              ars::Span<const DrawRequest> requests,
              const DrawCallbacks &callbacks = {});

    void
    draw(CommandBuffer *cmd, View *view, ars::Span<const DrawRequest> requests);

    // Use the view holding the drawer
    void draw(CommandBuffer *cmd, ars::Span<const DrawRequest> requests);

  private:
    void init_draw_id_billboard_alpha_clip();

    void draw(CommandBuffer *cmd,
              const glm::mat4 &P,
              const glm::mat4 &V,
              const Handle<Buffer> &view_transform_buffer,
              ars::Span<const DrawRequest> requests,
              const DrawCallbacks &callbacks);

    View *_view = nullptr;
    std::unique_ptr<GraphicsPipeline> _draw_id_billboard_alpha_clip_pipeline{};
};
} // namespace ars::render::vk