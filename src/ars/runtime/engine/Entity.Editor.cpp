#include "Entity.Editor.h"
#include <ars/runtime/core/gui/ImGui.h>

namespace ars::engine::editor {
Scene *HierarchyInspector::scene() const {
    return _scene;
}

void HierarchyInspector::set_scene(Scene *scene) {
    if (_scene == scene) {
        return;
    }
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

    bool opened =
        ImGui::TreeNodeEx(entity, flags, "%s", entity->name().c_str());

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

void HierarchyInspector::set_current_selected(Entity *entity) {
    _current_selected = entity;
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

    int imgui_id = 0;
    auto group = [&](auto &&func) {
        ImGui::PushID(imgui_id++);
        func();
        ImGui::PopID();
    };
    group([&]() {
        auto name = _entity->name();
        if (gui::input_text("Name", name)) {
            _entity->set_name(name);
        }
    });

    ImGui::Separator();
    group([&]() {
        auto xform = _entity->local_xform();
        if (gui::input_xform("Xform", xform)) {
            _entity->set_local_xform(xform);
        }
    });

    auto components = _entity->components();
    auto comp_count = components.size();
    std::vector<bool> to_remove(comp_count);
    for (size_t i = 0; i < comp_count; i++) {
        group([&]() {
            ImGui::Separator();
            auto comp = components[i];
            auto comp_name = comp->type().get_name().to_string();
            ImGui::Text("%s", comp_name.c_str());
            ImGui::SameLine();
            if (ImGui::Button("Delete")) {
                to_remove[i] = true;
            }
            group([&]() { comp->on_inspector(); });
        });
    }

    for (size_t i = 0; i < comp_count; i++) {
        if (to_remove[i]) {
            _entity->remove_component(components[i]->type());
        }
    }

    ImGui::Separator();

    group([&]() {
        const char *popup_key = "Create_Component_Popup";
        if (ImGui::Button("Add Component")) {
            ImGui::OpenPopup(popup_key);
        }
        if (ImGui::BeginPopup(popup_key)) {
            for (const auto &ty :
                 global_component_registry()->component_types) {
                auto &[ty_name, comp_reg] = ty;
                // Skip existing component
                if (_entity->component(comp_reg->type()) != nullptr) {
                    continue;
                }
                if (ImGui::MenuItem(ty_name.c_str())) {
                    _entity->add_component(comp_reg->type());
                }
            }
            ImGui::EndPopup();
        }
    });
}
} // namespace ars::engine::editor