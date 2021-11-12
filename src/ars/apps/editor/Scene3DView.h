#pragma once

#include <ars/runtime/engine/Entity.h>
#include <ars/runtime/render/IScene.h>

namespace ars::editor {
class Scene3DView {
  public:
    void set_scene(engine::Scene *scene);
    void set_selected(engine::Entity *entity);
    void set_view(render::IView *view);
    void set_framebuffer_scale(float scale);
    void on_imgui();

  private:
    void draw_selected_object_outline();

    engine::Scene *_scene = nullptr;
    render::IView *_view = nullptr;
    engine::Entity *_current_selected = nullptr;
    float _framebuffer_scale = 1.0f;
};
} // namespace ars::editor