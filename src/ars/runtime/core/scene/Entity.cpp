#include "Entity.h"
#include "../Log.h"
#include "../gui/ImGui.h"
#include <cstdlib>
#include <sstream>

namespace ars::scene {
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
        std::stringstream ss;
        ss << "Try to remove " << ty << " of Entity \"" << entity_name
           << "\" at index " << index << ", which is out of range";
        log_warn(ss.str());
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

IComponent *Entity::component(size_t index) const {
    return _components.at(index).get();
}

void Entity::remove_component(size_t index) {
    erase_at_index(_components, index, name(), "component");
}

void Entity::insert_component(std::unique_ptr<IComponent> component,
                              std::optional<size_t> index) {
    return insert_at_index(_components, std::move(component), index);
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
        set_local_xform(xform * parent()->world_xform().inverse());
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
    if (entity == _root) {
        log_error("If one what to delete the root of a scene, he should delete "
                  "the scene itself");
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
        destroy_entity(child);
    }

    entity->set_parent(nullptr);
    _entities.free(entity->id());
}

Entity *Scene::root() const {
    return _root;
}

Scene::Scene() {
    _root = create_entity();
    _root->set_name("ROOT");
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

Scene::~Scene() = default;

void IComponent::on_inspector() {
    gui::input_instance(*this);
}

ComponentRegistry *global_component_registry() {
    static ComponentRegistry reg{};
    return &reg;
}

void register_component_type_entry(
    const std::string &name, std::unique_ptr<IComponentRegistryEntry> entry) {
    global_component_registry()->component_types.emplace(name,
                                                         std::move(entry));
}
} // namespace ars::scene
