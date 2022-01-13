#pragma once

#include <ars/runtime/core/math/Transform.h>
#include <ars/runtime/core/misc/Macro.h>
#include <ars/runtime/core/misc/SoA.h>
#include <filesystem>
#include <memory>
#include <nlohmann/json.hpp>
#include <optional>
#include <rttr/registration>
#include <set>
#include <string>
#include <vector>

namespace ars::engine {
class Entity;
class RenderSystem;

class IComponent {
    RTTR_ENABLE();

  public:
    virtual ~IComponent() = default;
    [[nodiscard]] rttr::type type() const;

    virtual nlohmann::json serialize();
    virtual void deserialize(const nlohmann::json &js);
    virtual void on_inspector();

    virtual void init(Entity *entity) {}
    virtual void destroy() {}
};

namespace details {
template <typename T> struct ComponentAutoRegister {
    // noexcept silence some clang-tidy warning. The constructor is called
    // before the main function is start, the program halts if any exception is
    // thrown.
    ComponentAutoRegister() noexcept {
        T::register_component();
    }
};
} // namespace details

template <typename T> auto register_component(const std::string &name) {
    return rttr::registration::class_<T>(name).template constructor<>()(
        rttr::policy::ctor::as_raw_ptr);
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
    // Get all entities in the scene, including those not in the scene tree
    [[nodiscard]] std::vector<Entity *> entities() const;

    void update_cached_world_xform();

    // Call this in update
    void update();

    [[nodiscard]] RenderSystem *render_system() const;

  private:
    void
    update_cached_world_xform_impl(Entity *entity,
                                   const math::XformTRS<float> &parent_xform);
    void destroy_entity_impl(Entity *entity, bool can_destroy_root);

    Container _entities{};
    Entity *_root{};
    std::unique_ptr<RenderSystem> _render_system{};
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
    IComponent *add_component(const rttr::type &ty);

    template <typename T> [[nodiscard]] T *component() const {
        return dynamic_cast<T *>(component(rttr::type::get<T>()));
    }
    template <typename T> void remove_component() {
        remove_component(rttr::type::get<T>());
    }
    template <typename T> T *add_component() {
        return dynamic_cast<T *>(add_component(rttr::type::get<T>()));
    }

    [[nodiscard]] math::XformTRS<float> local_xform() const;
    void set_local_xform(const math::XformTRS<float> &xform);
    [[nodiscard]] math::XformTRS<float> world_xform() const;
    void set_world_xform(const math::XformTRS<float> &xform);

    // Value only valid after call to Scene::update_cached_world_xform
    [[nodiscard]] math::XformTRS<float> cached_world_xform() const;
    void set_cached_world_xform(const math::XformTRS<float> &xform);

    template <typename Func> void visit_preorder(Func &&func) {
        func(this);
        for (auto child : _children) {
            child->template visit_preorder(func);
        }
    }

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
} // namespace ars::engine
