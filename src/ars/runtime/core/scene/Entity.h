#pragma once

#include "../math/Transform.h"
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
    using Container = SoA<std::unique_ptr<Entity>>;

  public:
    using EntityId = Container::Id;

    Scene();
    ~Scene();
    Entity *create_entity();
    void destroy_entity(Entity *entity);

    [[nodiscard]] Entity *root() const;

  private:
    Container _entities{};
    Entity *_root{};
};

using EntityId = Scene::EntityId;

class Entity final {
  public:
    Entity(Scene *scene, EntityId id);

    [[nodiscard]] Scene *scene() const;
    [[nodiscard]] EntityId id() const;

    [[nodiscard]] std::string name() const;
    void set_name(const std::string &name);

    [[nodiscard]] Entity *parent() const;
    // By default, the entity is inserted at the end of the children list.
    // If parent == nullptr, the node is removed from the tree, but not
    // destroyed.
    void set_parent(Entity *parent, std::optional<size_t> index = std::nullopt);

    [[nodiscard]] size_t child_count() const;
    [[nodiscard]] Entity *child(size_t index) const;

    [[nodiscard]] size_t component_count() const;
    [[nodiscard]] IComponent *component(size_t index) const;
    void remove_component(size_t index);
    void insert_component(std::unique_ptr<IComponent> component,
                          std::optional<size_t> index = std::nullopt);

    [[nodiscard]] math::XformTRS<float> local_xform() const;
    void set_local_xform(const math::XformTRS<float> &xform);
    [[nodiscard]] math::XformTRS<float> world_xform() const;
    void set_world_xform(const math::XformTRS<float> &xform);

  private:
    math::XformTRS<float> _local_to_parent{};
    Scene *_scene{};
    EntityId _id{};
    std::string _name = "New Entity";
    Entity *_parent{};
    std::vector<Entity *> _children{};
    std::vector<std::unique_ptr<IComponent>> _components{};
};
} // namespace ars::scene
