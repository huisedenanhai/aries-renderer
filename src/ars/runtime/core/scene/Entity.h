#pragma once

#include "../math/Transform.h"
#include "../misc/SoA.h"
#include <memory>
#include <optional>
#include <rttr/registration>
#include <set>
#include <string>
#include <vector>

namespace ars::scene {
class Entity;

class IComponent {
    RTTR_ENABLE();

  public:
    virtual ~IComponent() = default;
    virtual rttr::type type() = 0;

    virtual void init(Entity *entity) {}
    virtual void destroy() {}
    virtual void on_inspector();
};

// Almost the same as RTTR_ENABLE, fix warning on some compiler with 'override'
#define ARS_COMPONENT_RTTR_ENABLE(...)                                         \
  public:                                                                      \
    rttr::type get_type() const override {                                     \
        return rttr::detail::get_type_from_instance(this);                     \
    }                                                                          \
    void *get_ptr() override {                                                 \
        return reinterpret_cast<void *>(this);                                 \
    }                                                                          \
    rttr::detail::derived_info get_derived_info() override {                   \
        return {reinterpret_cast<void *>(this),                                \
                rttr::detail::get_type_from_instance(this)};                   \
    }                                                                          \
    using base_class_list = TYPE_LIST(__VA_ARGS__);                            \
                                                                               \
  private:

#define ARS_COMPONENT(ty, ...)                                                 \
  public:                                                                      \
    rttr::type type() override {                                               \
        return rttr::type::get<ty>();                                          \
    }                                                                          \
    ARS_COMPONENT_RTTR_ENABLE(__VA_ARGS__);

#define ARS_REGISTER_COMPONENT(ty)                                             \
    RTTR_PLUGIN_REGISTRATION {                                                 \
        ty::register_component();                                              \
    }

class IComponentRegistryEntry {
  public:
    virtual ~IComponentRegistryEntry() = default;
    virtual std::unique_ptr<IComponent> create_instance() = 0;
    virtual rttr::type type() = 0;
};

class ComponentRegistry {
  public:
    std::unordered_map<std::string, std::unique_ptr<IComponentRegistryEntry>>
        component_types{};
};

template <typename T>
class ComponentRegistryEntry : public IComponentRegistryEntry {
  public:
    std::unique_ptr<IComponent> create_instance() override {
        return std::make_unique<T>();
    }

    rttr::type type() override {
        return rttr::type::get<T>();
    }
};

ComponentRegistry *global_component_registry();

void register_component_type_entry(
    const std::string &name, std::unique_ptr<IComponentRegistryEntry> entry);

template <typename T> void register_component_type(const std::string &name) {
    register_component_type_entry(
        name, std::make_unique<ComponentRegistryEntry<T>>());
}

template <typename T> auto register_component(const std::string &name) {
    register_component_type<T>(name);
    return rttr::registration::class_<T>(name).template constructor<>();
}

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

    void update_cached_world_xform();

  private:
    void
    update_cached_world_xform_impl(Entity *entity,
                                   const math::XformTRS<float> &parent_xform);
    void destroy_entity_impl(Entity *entity, bool can_destroy_root);

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
    [[nodiscard]] std::vector<IComponent *> components() const;
    [[nodiscard]] IComponent *component(const rttr::type &ty) const;
    void remove_component(const rttr::type &ty);
    void add_component(const rttr::type &ty);

    template <typename T> [[nodiscard]] IComponent *component() const {
        return component(rttr::type::get<T>());
    }
    template <typename T> void remove_component() {
        remove_component(rttr::type::get<T>());
    }
    template <typename T> void add_component() {
        add_component(rttr::type::get<T>());
    }

    [[nodiscard]] math::XformTRS<float> local_xform() const;
    void set_local_xform(const math::XformTRS<float> &xform);
    [[nodiscard]] math::XformTRS<float> world_xform() const;
    void set_world_xform(const math::XformTRS<float> &xform);

    // Value only valid after call to Scene::update_cached_world_xform
    [[nodiscard]] math::XformTRS<float> cached_world_xform() const;
    void set_cached_world_xform(const math::XformTRS<float> &xform);

  private:
    math::XformTRS<float> _local_to_parent{};
    math::XformTRS<float> _local_to_world_cached{};
    Scene *_scene{};
    EntityId _id{};
    std::string _name = "New Entity";
    Entity *_parent{};
    std::vector<Entity *> _children{};
    std::map<rttr::type, std::unique_ptr<IComponent>> _components{};
};
} // namespace ars::scene
