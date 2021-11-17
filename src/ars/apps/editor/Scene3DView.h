#pragma once

#include <ars/runtime/engine/Entity.h>
#include <ars/runtime/render/IScene.h>
#include <ars/runtime/render/IWindow.h>
#include <imgui/imgui.h>
#include <imguizmo/ImGuizmo.h>

namespace ars::editor {
struct Scene3DViewState {
    ImGuizmo::OPERATION gizmo_op = ImGuizmo::UNIVERSAL;
    ImGuizmo::MODE gizmo_mode = ImGuizmo::WORLD;
    bool enable_translate_snap = false;
    float translate_snap = 0.1f;
    bool enable_scale_snap = false;
    float scale_snap = 0.1f;
    bool enable_angle_snap = false;
    float angle_snap = 5.0f;

    float focus_distance = 2.0f;

    bool show_ground_wire_grid = true;
    float ground_wire_grid_spacing = 0.1f;
    glm::vec4 ground_wire_grid_color{0.2f, 0.2f, 0.2f, 0.9f};
};

void scene_3d_view(Scene3DViewState &state,
                   render::IWindow *window,
                   engine::Scene *scene,
                   render::IView *view,
                   float framebuffer_scale,
                   engine::Entity *&current_selected);
} // namespace ars::editor