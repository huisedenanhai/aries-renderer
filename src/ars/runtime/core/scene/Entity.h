#pragma once

#include "../misc/SoA.h"
#include <memory>
#include <optional>
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
    Scene() = default;
    ~Scene();
    Entity *create_entity();
    void destroy_entity(Entity *entity);

  private:
    Container _entities{};
};

class Entity {
  public:
    std::string name() const;
    void set_name(const std::string &name);

    Entity *parent() const;

    size_t child_count() const;
    Entity *child(size_t index) const;
    void remove_child(size_t index);
    void remove_child(Entity *entity);
    void insert_child(Entity *entity,
                      std::optional<size_t> index = std::nullopt);

    size_t component_count() const;
    IComponent *component(size_t index) const;
    void remove_component(size_t index);
    void insert_component(std::unique_ptr<IComponent> component,
                          std::optional<size_t> index = std::nullopt);

  private:
    std::string _name = "New Entity";
    Entity *_parent{};
    std::vector<Entity *> _children{};
    std::vector<std::unique_ptr<IComponent>> _components{};
};

struct Scene::EntityDerived : public Entity {
    Scene *scene{};
    Scene::EntityId id{};
};

} // namespace ars::scene
