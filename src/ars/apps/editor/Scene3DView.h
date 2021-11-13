#pragma once

#include <ars/runtime/engine/Entity.h>
#include <ars/runtime/render/IScene.h>

namespace ars::editor {
void scene_3d_view(engine::Scene *scene,
                   render::IView *view,
                   float framebuffer_scale,
                   engine::Entity *&current_selected);
} // namespace ars::editor