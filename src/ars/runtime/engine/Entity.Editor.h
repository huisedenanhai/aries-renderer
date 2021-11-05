#pragma once

#include "Entity.h"

namespace ars::engine::editor {
class HierarchyInspector {
  public:
    [[nodiscard]] Scene *scene() const;
    void set_scene(Scene *scene);

    [[nodiscard]] Entity *current_selected() const;

    void on_imgui();

  private:
    Entity *create_entity();
    void draw_entity(Entity *entity);
    // need to call ImGui::TreePop after this method returns true
    bool begin_entity_tree_node(Entity *entity);
    void right_click_pop_up();

    Scene *_scene{};
    Entity *_current_selected{};
};

class EntityInspector {
  public:
    [[nodiscard]] Entity *entity() const;
    void set_entity(Entity *entity);

    void on_imgui();

  private:
    Entity *_entity = nullptr;
};
} // namespace ars::engine::editor