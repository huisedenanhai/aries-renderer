#include "Entity.Editor.h"
#include "../gui/ImGui.h"

namespace ars::scene::editor {
Scene *HierarchyInspector::scene() const {
    return _scene;
}

void HierarchyInspector::set_scene(Scene *scene) {
    _scene = scene;
    _current_selected = nullptr;
}

Entity *HierarchyInspector::current_selected() const {
    return _current_selected;
}

void HierarchyInspector::on_imgui() {
    if (_scene == nullptr) {
        return;
    }
    draw_entity(_scene->root());
    if (ImGui::IsWindowHovered() && !ImGui::IsAnyItemHovered()) {
        if (ImGui::IsMouseReleased(ImGuiMouseButton_Left) ||
            ImGui::IsMouseReleased(ImGuiMouseButton_Right)) {
            _current_selected = nullptr;
        }
    }
    right_click_pop_up();
}

Entity *HierarchyInspector::create_entity() {
    return _scene->create_entity();
}

void HierarchyInspector::draw_entity(Entity *entity) {
    if (begin_entity_tree_node(entity)) {
        auto child_count = entity->child_count();
        for (size_t i = 0; i < child_count; i++) {
            draw_entity(entity->child(i));
        }
        ImGui::TreePop();
    }
}

bool HierarchyInspector::begin_entity_tree_node(Entity *entity) {
    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnDoubleClick;
    flags |= entity == _current_selected ? ImGuiTreeNodeFlags_Selected : 0;
    auto is_leaf = entity->child_count() == 0;
    flags |= is_leaf ? ImGuiTreeNodeFlags_Leaf : 0;

    bool opened = ImGui::TreeNodeEx(entity->name().c_str(), flags);

    if (ImGui::IsItemClicked(ImGuiMouseButton_Left) ||
        ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
        _current_selected = entity;
    }

    return opened;
}

void HierarchyInspector::right_click_pop_up() {
    assert(_scene != nullptr);
    if (ImGui::BeginPopupContextWindow("Entity Popup")) {
        if (ImGui::MenuItem("Create Empty Entity")) {
            auto parent = _current_selected == nullptr ? _scene->root()
                                                       : _current_selected;
            // Insert at position zero so the mouse needs less movement to be
            // hovered on the new entity.
            auto new_entity = create_entity();
            new_entity->set_parent(parent, 0);
        }
        if (_current_selected != nullptr &&
            _current_selected != _scene->root()) {
            if (ImGui::MenuItem("Delete")) {
                _scene->destroy_entity(_current_selected);
                _current_selected = nullptr;
            }
        }
        ImGui::EndPopup();
    }
}

Entity *EntityInspector::entity() const {
    return _entity;
}

void EntityInspector::set_entity(Entity *entity) {
    _entity = entity;
}

void EntityInspector::on_imgui() {
    if (_entity == nullptr) {
        return;
    }
    auto name = _entity->name();
    if (gui::input_text("Name", name)) {
        _entity->set_name(name);
    }
}
} // namespace ars::scene::editor