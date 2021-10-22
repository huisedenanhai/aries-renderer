#pragma once

#include "../misc/SoA.h"
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <vector>

namespace ars::scene {
class IComponent {};

class Entity;

class Scene {
  private:
    struct EntityDerived;

    using Container = SoA<std::unique_ptr<EntityDerived>>;
    using EntityId = Container::Id;

  public:
    Scene();
    ~Scene();
    Entity *create_entity();
    void destroy_entity(Entity *entity);

    Entity *root() const;

  private:
    Container _entities{};
    Entity *_root{};
};

class Entity {
  public:
    explicit Entity(Scene *scene);

    Scene *scene() const;

    [[nodiscard]] std::string name() const;
    void set_name(const std::string &name);

    [[nodiscard]] Entity *parent() const;
    // By default, the entity is inserted at the end of the children list.
    // If parent == nullptr, the node is removed from the tree
    void set_parent(Entity *parent, std::optional<size_t> index = std::nullopt);

    [[nodiscard]] size_t child_count() const;
    [[nodiscard]] Entity *child(size_t index) const;

    [[nodiscard]] size_t component_count() const;
    [[nodiscard]] IComponent *component(size_t index) const;
    void remove_component(size_t index);
    void insert_component(std::unique_ptr<IComponent> component,
                          std::optional<size_t> index = std::nullopt);

  private:
    Scene *_scene{};
    std::string _name = "New Entity";
    Entity *_parent{};
    std::vector<Entity *> _children{};
    std::vector<std::unique_ptr<IComponent>> _components{};
};

struct Scene::EntityDerived : public Entity {
    using Entity::Entity;

    Scene::EntityId id{};
};

} // namespace ars::scene
