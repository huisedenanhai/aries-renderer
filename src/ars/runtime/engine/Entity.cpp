#include "Entity.h"
#include "Engine.h"
#include "Spawn.h"
#include "components/RenderSystem.h"
#include "gui/ImGui.h"
#include <ars/runtime/core/Log.h>
#include <ars/runtime/core/Serde.h>
#include <cstdlib>
#include <fstream>

namespace ars::engine {
std::string Entity::name() const {
    return _name;
}

void Entity::set_name(const std::string &name) {
    _name = name;
}

namespace {
template <typename T>
void insert_at_index(std::vector<std::remove_reference_t<T>> &vec,
                     T &&v,
                     std::optional<size_t> index) {
    auto i = std::min(index.value_or(vec.size()), vec.size());
    auto it = vec.begin() + i;
    vec.insert(it, std::forward<T>(v));
}

template <typename T>
T erase_at_index(std::vector<T> &vec,
                 size_t index,
                 const std::string &entity_name,
                 const std::string &ty) {
    if (index >= vec.size()) {
        ARS_LOG_WARN("Try to remove {} of Entity \"{}\" at index {}, which is "
                     "out of range",
                     ty,
                     entity_name,
                     index);
        return {};
    }
    auto old = std::move(vec[index]);
    vec.erase(vec.begin() + index);
    return old;
}
} // namespace

size_t Entity::child_count() const {
    return _children.size();
}

Entity *Entity::child(size_t index) const {
    return _children.at(index);
}

void Entity::set_parent(Entity *parent, std::optional<size_t> index) {
    if (_parent != nullptr) {
        auto &p_children = _parent->_children;
        p_children.erase(
            std::remove(p_children.begin(), p_children.end(), this),
            p_children.end());
    }
    if (parent != nullptr) {
        insert_at_index(parent->_children, this, index);
    }
    _parent = parent;
}

size_t Entity::component_count() const {
    return _components.size();
}

Entity *Entity::parent() const {
    return _parent;
}

Entity::Entity(Scene *scene, EntityId id) : _scene(scene), _id(id) {
    assert(scene != nullptr);
}

Scene *Entity::scene() const {
    return _scene;
}

EntityId Entity::id() const {
    return _id;
}

math::XformTRS<float> Entity::local_xform() const {
    return _local_to_parent;
}

math::XformTRS<float> Entity::world_xform() const {
    if (parent() != nullptr) {
        return parent()->world_xform() * _local_to_parent;
    }
    return _local_to_parent;
}

void Entity::set_local_xform(const math::XformTRS<float> &xform) {
    _local_to_parent = xform;
}

void Entity::set_world_xform(const math::XformTRS<float> &xform) {
    if (parent() != nullptr) {
        set_local_xform(parent()->world_xform().inverse() * xform);
        return;
    }
    set_local_xform(xform);
}

math::XformTRS<float> Entity::cached_world_xform() const {
    return _local_to_world_cached;
}

void Entity::set_cached_world_xform(const math::XformTRS<float> &xform) {
    _local_to_world_cached = xform;
}

std::vector<IComponent *> Entity::components() const {
    std::vector<IComponent *> comps{};
    comps.reserve(_components.size());
    for (auto &c : _components) {
        comps.push_back(c.second.get());
    }
    return comps;
}

IComponent *Entity::component(const rttr::type &ty) const {
    auto it = _components.find(ty);
    if (it == _components.end()) {
        return nullptr;
    }
    return it->second.get();
}

void Entity::remove_component(const rttr::type &ty) {
    auto it = _components.find(ty);
    if (it == _components.end()) {
        return;
    }

    auto comp = it->second.get();
    comp->destroy();
    _components.erase(it);
}

IComponent *Entity::add_component(const rttr::type &ty) {
    auto ty_name = ty.get_name().to_string();
    if (!ty.is_derived_from<IComponent>()) {
        ARS_LOG_ERROR(
            "Try to add type \"{}\", which does not derived from IComponent.",
            ty_name);
        return nullptr;
    }

    if (_components.find(ty) != _components.end()) {
        ARS_LOG_WARN("Component \"{}\" already exists on entity \"{}\", add it "
                     "twice will do nothing",
                     ty_name,
                     name());
        return nullptr;
    }

    bool success = false;
    std::unique_ptr<IComponent> comp(
        ty.create().convert<IComponent *>(&success));
    if (!success) {
        ARS_LOG_ERROR(
            "Failed to create instance of {}: the component type "
            "should be registered with method ars::engine::register_component",
            ty_name,
            name());
    }

    auto comp_ptr = comp.get();
    // Notice the initialization order
    _components[ty] = std::move(comp);
    comp_ptr->init(this);
    return comp_ptr;
}

Entity *Scene::create_entity() {
    auto id = _entities.alloc();
    auto &entity = _entities.get<std::unique_ptr<Entity>>(id);
    entity = std::make_unique<Entity>(this, id);
    // First call to create_entity happens in Scene construct, root() will
    // return nullptr
    entity->set_parent(root());

    return entity.get();
}

void Scene::destroy_entity(Entity *entity) {
    destroy_entity_impl(entity, false);
}

Entity *Scene::root() const {
    return _root;
}

Scene::Scene() {
    _root = create_entity();
    _root->set_name("ROOT");
    _render_system = std::make_unique<RenderSystem>(render_context());
}

void Scene::update_cached_world_xform() {
    update_cached_world_xform_impl(root(), {});
}

void Scene::update_cached_world_xform_impl(
    Entity *entity, const math::XformTRS<float> &parent_xform) {
    auto w_xform = parent_xform * entity->local_xform();
    entity->set_cached_world_xform(w_xform);

    for (size_t i = 0; i < entity->child_count(); i++) {
        update_cached_world_xform_impl(entity->child(i), w_xform);
    }
}

Scene::~Scene() {
    destroy_entity_impl(_root, true);
}

void Scene::destroy_entity_impl(Entity *entity, bool can_destroy_root) {
    if (!can_destroy_root && entity == _root) {
        ARS_LOG_ERROR(
            "Try to delete the root of the scene. If one what to delete "
            "the root of a scene, he should delete the scene itself");
        return;
    }
    if (entity == nullptr) {
        return;
    }

    assert(entity->scene() == this);

    // Copy children as the destroy_entity method modifies children list of the
    // entity
    std::vector<Entity *> children{};
    children.reserve(entity->child_count());
    for (size_t i = 0; i < entity->child_count(); i++) {
        children.push_back(entity->child(i));
    }

    for (auto child : children) {
        // Children should never be the root
        destroy_entity_impl(child, false);
    }

    for (auto comp : entity->components()) {
        entity->remove_component(comp->type());
    }

    entity->set_parent(nullptr);
    _entities.free(entity->id());
}

RenderSystem *Scene::render_system() const {
    return _render_system.get();
}

void Scene::update() {
    update_cached_world_xform();
    render_system()->update();
}

void IComponent::on_inspector() {
    gui::input_instance(*this);
}

nlohmann::json IComponent::serialize() {
    return rttr::variant(this);
}

void IComponent::deserialize(const nlohmann::json &js) {
    auto inst = rttr::variant(this);
    js.get_to(inst);
}

rttr::type IComponent::type() const {
    return get_type();
}
} // namespace ars::engine
