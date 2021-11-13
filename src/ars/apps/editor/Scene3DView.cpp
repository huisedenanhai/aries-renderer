#include "Scene3DView.h"
#include <ars/runtime/engine/components/RenderSystem.h>
#include <imgui/imgui.h>

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
        view->overlay()->draw_outline(0,
                                      current_selected->cached_world_xform(),
                                      mesh_renderer->primitive(i)->mesh());
    }
}

void click_selection(render::IView *view,
                     float framebuffer_scale,
                     engine::Entity *&current_selected) {
    if (!ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
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
} // namespace

void scene_3d_view(engine::Scene *scene,
                   render::IView *view,
                   float framebuffer_scale,
                   engine::Entity *&current_selected) {
    if (view == nullptr || scene == nullptr) {
        return;
    }

    draw_selected_object_outline(view, current_selected);

    auto size = ImGui::GetContentRegionAvail();
    render::Extent2D framebuffer_size = {
        static_cast<uint32_t>(size.x * framebuffer_scale),
        static_cast<uint32_t>(size.y * framebuffer_scale),
    };

    view->set_size(framebuffer_size);
    view->render();

    ImGui::Image(view->get_color_texture(), size);

    click_selection(view, framebuffer_scale, current_selected);
}
} // namespace ars::editor
