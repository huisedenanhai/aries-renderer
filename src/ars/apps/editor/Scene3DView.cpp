#include "Scene3DView.h"
#include <ars/runtime/engine/components/RenderSystem.h>
#include <imgui/imgui.h>

namespace ars::editor {
void Scene3DView::set_scene(engine::Scene *scene) {
    if (_scene == scene) {
        return;
    }
    _scene = scene;
    _view = nullptr;
    _current_selected = nullptr;
}

void Scene3DView::set_selected(engine::Entity *entity) {
    _current_selected = entity;
}

void Scene3DView::set_view(render::IView *view) {
    _view = view;
}

void Scene3DView::on_imgui() {
    if (_view == nullptr || _scene == nullptr) {
        return;
    }

    draw_selected_object_outline();

    auto size = ImGui::GetContentRegionAvail();
    render::Extent2D framebuffer_size = {
        static_cast<uint32_t>(size.x * _framebuffer_scale),
        static_cast<uint32_t>(size.y * _framebuffer_scale),
    };

    _view->set_size(framebuffer_size);
    _view->render();

    ImGui::Image(_view->get_color_texture(), size);
}

void Scene3DView::set_framebuffer_scale(float scale) {
    _framebuffer_scale = scale;
}

void Scene3DView::draw_selected_object_outline() {
    if (_current_selected == nullptr) {
        return;
    }
    auto mesh_renderer =
        _current_selected->component<ars::engine::MeshRenderer>();
    if (mesh_renderer == nullptr) {
        return;
    }
    for (int i = 0; i < mesh_renderer->primitive_count(); i++) {
        _view->overlay()->draw_outline(0,
                                       _current_selected->cached_world_xform(),
                                       mesh_renderer->primitive(i)->mesh());
    }
}
} // namespace ars::editor
