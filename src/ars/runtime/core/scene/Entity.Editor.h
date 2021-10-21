#pragma once

#include "Entity.h"

namespace ars::scene::editor {
class HierarchyInspector {
  public:
    Entity *root() const;
    void set_root(Entity *root);

    Entity *current_selected() const;

    void on_imgui();

  private:
    Entity *create_entity();
    void draw_entity(Entity *entity);
    // need to call ImGui::TreePop after this method returns true
    bool begin_entity_tree_node(Entity *entity);
    void right_click_pop_up();

    Scene _scene{};
    Entity *_root;
    Entity *_current_selected{};
};

class EntityInspector {};
} // namespace ars::scene::editor