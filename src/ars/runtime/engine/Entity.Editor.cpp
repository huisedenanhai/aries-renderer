#include "Entity.Editor.h"
#include <ars/runtime/core/gui/ImGui.h>

namespace ars::engine::editor {
namespace {
bool begin_entity_tree_node(Entity *entity, Entity *&current_selected) {
    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnDoubleClick;
    flags |= entity == current_selected ? ImGuiTreeNodeFlags_Selected : 0;
    auto is_leaf = entity->child_count() == 0;
    flags |= is_leaf ? ImGuiTreeNodeFlags_Leaf : 0;

    bool opened =
        ImGui::TreeNodeEx(entity, flags, "%s", entity->name().c_str());

    if (ImGui::IsItemClicked(ImGuiMouseButton_Left) ||
        ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
        current_selected = entity;
    }

    return opened;
}

void draw_entity(Entity *entity, Entity *&current_selected) {
    if (begin_entity_tree_node(entity, current_selected)) {
        auto child_count = entity->child_count();
        for (size_t i = 0; i < child_count; i++) {
            draw_entity(entity->child(i), current_selected);
        }
        ImGui::TreePop();
    }
}

void right_click_pop_up(Scene *scene, Entity *&current_selected) {
    assert(scene != nullptr);
    if (ImGui::BeginPopupContextWindow("Entity Popup")) {
        if (ImGui::MenuItem("Create Empty Entity")) {
            auto parent =
                current_selected == nullptr ? scene->root() : current_selected;
            // Insert at position zero so the mouse needs less movement to be
            // hovered on the new entity.
            auto new_entity = scene->create_entity();
            new_entity->set_parent(parent, 0);
            current_selected = new_entity;
        }
        if (current_selected != nullptr && current_selected != scene->root()) {
            if (ImGui::MenuItem("Delete")) {
                scene->destroy_entity(current_selected);
                current_selected = nullptr;
            }
        }
        ImGui::EndPopup();
    }
}
} // namespace

void entity_inspector(Entity *entity) {
    if (entity == nullptr) {
        return;
    }

    int imgui_id = 0;
    auto group = [&](auto &&func) {
        ImGui::PushID(imgui_id++);
        func();
        ImGui::PopID();
    };
    group([&]() {
        auto name = entity->name();
        if (gui::input_text("Name", name)) {
            entity->set_name(name);
        }
    });

    ImGui::Separator();
    group([&]() {
        auto xform = entity->local_xform();
        if (gui::input_xform("Xform", xform)) {
            entity->set_local_xform(xform);
        }
    });

    auto components = entity->components();
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
            entity->remove_component(components[i]->type());
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
                if (entity->component(comp_reg->type()) != nullptr) {
                    continue;
                }
                if (ImGui::MenuItem(ty_name.c_str())) {
                    entity->add_component(comp_reg->type());
                }
            }
            ImGui::EndPopup();
        }
    });
}

void hierarchy_inspector(Scene *scene, Entity *&current_selected) {
    if (scene == nullptr) {
        return;
    }
    draw_entity(scene->root(), current_selected);
    if (ImGui::IsWindowHovered() && !ImGui::IsAnyItemHovered()) {
        if (ImGui::IsMouseReleased(ImGuiMouseButton_Left) ||
            ImGui::IsMouseReleased(ImGuiMouseButton_Right)) {
            current_selected = nullptr;
        }
    }
    right_click_pop_up(scene, current_selected);
}

} // namespace ars::engine::editor