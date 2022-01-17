#include "Scene3DView.h"
#include <ars/runtime/core/input/Keyboard.h>
#include <ars/runtime/engine/components/RenderSystem.h>
#include <ars/runtime/engine/gui/ImGui.h>
#include <ars/runtime/render/IEnvironment.h>
#include <ars/runtime/render/IMesh.h>
#include <imgui/imgui.h>
#include <imguizmo/ImGuizmo.h>

namespace ars::editor {
namespace {
void draw_selected_object_outline(render::IView *view,
                                  engine::Entity *current_selected) {
    if (current_selected == nullptr) {
        return;
    }
    auto mesh_renderer =
        current_selected->component<ars::engine::MeshRenderer>();
    if (mesh_renderer == nullptr) {
        return;
    }
    for (int i = 0; i < mesh_renderer->primitive_count(); i++) {
        auto overlay = view->overlay();
        auto mesh = mesh_renderer->primitive(i)->mesh();
        if (mesh == nullptr) {
            continue;
        }
        overlay->draw_outline(0, current_selected->cached_world_xform(), mesh);
        auto aabb = mesh->aabb();
        overlay->draw_wire_box(current_selected->cached_world_xform(),
                               aabb.center(),
                               aabb.extent(),
                               {0.0f, 1.0f, 0.0f, 1.0f});
    }
}

void click_selection(render::IView *view,
                     float framebuffer_scale,
                     engine::Entity *&current_selected) {
    if (!ImGui::IsWindowFocused() ||
        !ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
        return;
    }
    auto mouse_pos = ImGui::GetMousePos();
    auto window_pos = ImGui::GetWindowPos();
    auto region_min = ImGui::GetWindowContentRegionMin();
    ImVec2 mouse_pos_local = {
        mouse_pos.x - window_pos.x - region_min.x,
        mouse_pos.y - window_pos.y - region_min.y,
    };

    auto selection = view->query_selection(
        static_cast<uint32_t>(framebuffer_scale * mouse_pos_local.x),
        static_cast<uint32_t>(framebuffer_scale * mouse_pos_local.y),
        1,
        1);
    if (!selection.empty()) {
        current_selected =
            reinterpret_cast<ars::engine::Entity *>(selection[0]);
    }
}

void transform_gizmo(Scene3DViewState &state,
                     render::IView *view,
                     engine::Entity *current_selected) {
    if (current_selected == nullptr) {
        return;
    }

    ImGuiIO &io = ImGui::GetIO();
    auto win_pos = ImGui::GetWindowPos();
    auto region_min = ImGui::GetWindowContentRegionMin();
    auto region_max = ImGui::GetWindowContentRegionMax();

    ImGuizmo::SetRect(win_pos.x + region_min.x,
                      win_pos.y + region_min.y,
                      region_max.x - region_min.x,
                      region_max.y - region_min.y);
    ImGuizmo::SetDrawlist(ImGui::GetWindowDrawList());
    auto proj_matrix = view->camera().projection_matrix(
        static_cast<float>(view->size().width) /
        static_cast<float>(view->size().height));
    auto model_matrix = current_selected->world_xform().matrix();
    auto view_matrix = glm::inverse(view->xform().matrix_no_scale());
    auto view_matrix_imguizmo =
        glm::mat4(-1, 0, 0, 0, 0, 1, 0, 0, 0, 0, -1, 0, 0, 0, 0, 1) *
        view_matrix;
    float translate_snap[3] = {
        state.translate_snap,
        state.translate_snap,
        state.translate_snap,
    };
    if (ImGuizmo::Manipulate(
            &view_matrix_imguizmo[0][0],
            &proj_matrix[0][0],
            state.gizmo_op,
            state.gizmo_mode,
            &model_matrix[0][0],
            nullptr,
            state.enable_translate_snap ? translate_snap : nullptr,
            state.enable_angle_snap ? &state.angle_snap : nullptr,
            state.enable_scale_snap ? &state.scale_snap : nullptr)) {
        current_selected->set_world_xform(
            ars::math::XformTRS<float>(model_matrix));
    }
}

void gizmo_menu(Scene3DViewState &state, render::IView *view) {
    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("Gizmo")) {
            ImGui::Text("Operation");
            bool translate_enabled = (state.gizmo_op & ImGuizmo::TRANSLATE);
            ImGui::Checkbox("Translate", &translate_enabled);
            bool rotate_enabled = (state.gizmo_op & ImGuizmo::ROTATE);
            ImGui::Checkbox("Rotate", &rotate_enabled);
            bool scale_enabled = (state.gizmo_op & ImGuizmo::SCALEU);
            ImGui::Checkbox("Scale", &scale_enabled);

            ImGuizmo::OPERATION op{};
            if (translate_enabled) {
                op = op | ImGuizmo::TRANSLATE;
            }
            if (rotate_enabled) {
                op = op | ImGuizmo::ROTATE;
            }
            if (scale_enabled) {
                op = op | ImGuizmo::SCALEU;
            }

            state.gizmo_op = op;

            int mode = state.gizmo_mode;
            const char *mode_names[2] = {"Local", "World"};
            if (ImGui::Combo("Mode",
                             &mode,
                             mode_names,
                             static_cast<int>(std::size(mode_names)))) {
                state.gizmo_mode = static_cast<ImGuizmo::MODE>(mode);
            }

            int id = 0;
            auto checkbox_float_input =
                [&](const char *label, bool *enabled, float *value) {
                    ImGui::PushID(id++);
                    ImGui::Checkbox("", enabled);
                    ImGui::SameLine();
                    ImGui::InputFloat(label, value);
                    ImGui::PopID();
                };
            checkbox_float_input("Translate Snap",
                                 &state.enable_translate_snap,
                                 &state.translate_snap);
            checkbox_float_input(
                "Angle Snap", &state.enable_angle_snap, &state.angle_snap);
            checkbox_float_input(
                "Scale Snap", &state.enable_scale_snap, &state.scale_snap);

            ImGui::Separator();
            ImGui::Text("Ground Wire Grid");
            ImGui::PushID(id++);
            ImGui::Checkbox("Show", &state.show_ground_wire_grid);
            ImGui::InputFloat("Spacing", &state.ground_wire_grid_spacing);
            gui::input_color4("Color", state.ground_wire_grid_color);
            ImGui::PopID();

            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Render")) {
            auto &strength = state.env_radiance_strength;
            auto env = view->environment();
            auto color = env->radiance() / std::max(0.01f, strength);
            gui::input_color3("Env Color", color);
            ImGui::InputFloat("Env Strength", &strength);
            strength = std::max(0.0f, strength);
            env->set_radiance(color * state.env_radiance_strength);
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }
}

void orbit_camera(Scene3DViewState &state, render::IView *view) {
    auto xform = view->xform();
    auto center = xform.translation() + xform.forward() * state.focus_distance;
    xform.set_translation(-xform.forward() * state.focus_distance);

    auto delta = ImGui::GetIO().MouseDelta;
    auto qx = glm::angleAxis(glm::radians(-delta.y), xform.right());
    auto qy =
        glm::angleAxis(glm::radians(-delta.x), glm::vec3(0.0f, 1.0f, 0.0f));

    xform = math::XformTRS<float>::from_rotation(glm::cross(qy, qx)) * xform;
    xform.set_translation(center - xform.forward() * state.focus_distance);

    view->set_xform(xform);
}

void pan_camera(Scene3DViewState &state,
                float framebuffer_scale,
                render::IView *view) {
    auto delta = ImGui::GetIO().MouseDelta;
    delta.x *= -framebuffer_scale;
    delta.y *= -framebuffer_scale;
    auto size = view->size();
    auto delta_hclip =
        2.0f * glm::vec2(delta.x / static_cast<float>(size.width),
                         delta.y / static_cast<float>(size.height));

    auto proj_mat = view->projection_matrix();
    auto focus_center_hclip =
        proj_mat * glm::vec4(0, 0, -state.focus_distance, 1.0f);
    auto focus_center_z = focus_center_hclip.z / focus_center_hclip.w;

    auto delta_view =
        glm::inverse(proj_mat) * glm::vec4(delta_hclip, focus_center_z, 1.0f);
    auto xform = view->xform();
    auto trans = xform.translation();
    trans += delta_view.x / delta_view.w * xform.right();
    trans += delta_view.y / delta_view.w * xform.up();
    xform.set_translation(trans);
    view->set_xform(xform);
}

void zoom_camera(Scene3DViewState &state, render::IView *view) {
    auto scroll = ImGui::GetIO().MouseWheel;
    if (scroll == 0) {
        return;
    }
    auto sensitivity = 0.1f;
    auto scale = std::clamp(scroll * sensitivity + 1.0f, 0.1f, 2.0f);
    auto distance = scale * state.focus_distance;
    auto delta = distance - state.focus_distance;
    auto xform = view->xform();
    auto t = xform.translation();
    t -= delta * xform.forward();
    xform.set_translation(t);
    view->set_xform(xform);
    state.focus_distance = distance;
}

float estimate_radius(engine::Entity *entity, glm::vec3 &center) {
    assert(entity != nullptr);
    auto mesh_renderer = entity->component<engine::MeshRenderer>();
    auto xform = entity->cached_world_xform();
    auto mat = xform.matrix();
    if (mesh_renderer != nullptr && mesh_renderer->primitive_count() > 0) {
        auto combined_aabb = math::transform_aabb(
            mat, mesh_renderer->primitive(0)->mesh()->aabb());
        for (int i = 1; i < mesh_renderer->primitive_count(); i++) {
            auto mesh = mesh_renderer->primitive(i)->mesh();
            auto aabb = math::transform_aabb(mat, mesh->aabb());
            combined_aabb.extend_aabb(aabb);
        }
        // The gizmos should also be inside the view when focus to object
        combined_aabb.extend_point(xform.translation());
        center = combined_aabb.center();
        return glm::length(combined_aabb.extent()) * 0.5f;
    }

    center = xform.translation();
    return 0.2f;
}

void control_view_camera(Scene3DViewState &state,
                         render::IWindow *window,
                         float framebuffer_scale,
                         render::IView *view,
                         engine::Entity *current_selected) {
    if (ImGui::IsMouseDragging(ImGuiMouseButton_Right)) {
        if (window->keyboard()->is_holding(input::Key::LeftShift) ||
            window->keyboard()->is_holding(input::Key::RightShift)) {
            pan_camera(state, framebuffer_scale, view);
        } else {
            orbit_camera(state, view);
        }
    } else {
        if (ImGui::IsWindowHovered()) {
            zoom_camera(state, view);
        }
        if (ImGui::IsWindowFocused()) {
            if (window->keyboard()->is_pressed(input::Key::F)) {
                focus_camera(state, view, current_selected);
            }
        }
    }
}

void draw_ground_wire_frame(Scene3DViewState &state, render::IView *view) {
    if (!state.show_ground_wire_grid) {
        return;
    }
    auto overlay = view->overlay();
    int count = 20;
    auto half_length =
        static_cast<float>(count) * state.ground_wire_grid_spacing;
    for (int x = -count; x <= count; x++) {
        auto fx = static_cast<float>(x) * state.ground_wire_grid_spacing;
        overlay->draw_line({fx, 0.0f, -half_length},
                           {fx, 0.0f, half_length},
                           state.ground_wire_grid_color);
        overlay->draw_line({-half_length, 0.0f, fx},
                           {half_length, 0.0f, fx},
                           state.ground_wire_grid_color);
    }
}
} // namespace

void focus_camera(Scene3DViewState &state,
                  render::IView *view,
                  engine::Entity *target) {
    if (target == nullptr) {
        return;
    }
    glm::vec3 center;
    auto radius = estimate_radius(target, center);
    auto xform = view->xform();
    state.focus_distance = radius * 4.0f;
    auto t = center - state.focus_distance * xform.forward();
    xform.set_translation(t);
    view->set_xform(xform);
}

void scene_3d_view(Scene3DViewState &state,
                   render::IWindow *window,
                   engine::Scene *scene,
                   render::IView *view,
                   float framebuffer_scale,
                   engine::Entity *&current_selected) {
    if (view == nullptr || scene == nullptr) {
        return;
    }

    gizmo_menu(state, view);

    draw_selected_object_outline(view, current_selected);
    draw_ground_wire_frame(state, view);

    auto size = ImGui::GetContentRegionAvail();
    render::Extent2D framebuffer_size = {
        static_cast<uint32_t>(size.x * framebuffer_scale),
        static_cast<uint32_t>(size.y * framebuffer_scale),
    };

    view->set_size(framebuffer_size);
    view->render();

    ImGui::Image(view->get_color_texture()->handle(), size);

    transform_gizmo(state, view, current_selected);
    if (!ImGuizmo::IsUsing() && !ImGuizmo::IsOver()) {
        click_selection(view, framebuffer_scale, current_selected);
    }
    control_view_camera(
        state, window, framebuffer_scale, view, current_selected);
}
} // namespace ars::editor
