#include "Entity.h"
#include "../Log.h"
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
    auto it = vec.begin() + static_cast<ssize_t>(i);
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
    vec.erase(vec.begin() + static_cast<ssize_t>(index));
    return old;
}
} // namespace

size_t Entity::child_count() const {
    return _children.size();
}

Entity *Entity::child(size_t index) const {
    return _children.at(index);
}

void Entity::remove_child(size_t index) {
    auto old = erase_at_index(_children, index, name(), "child");
    if (old != nullptr) {
        old->_parent = nullptr;
    }
}

void Entity::insert_child(Entity *entity, std::optional<size_t> index) {
    if (entity->parent() != nullptr) {
        entity->parent()->remove_child(entity);
    }
    insert_at_index(_children, entity, index);
    entity->_parent = this;
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

void Entity::remove_child(Entity *entity) {
    auto it = std::find(_children.begin(), _children.end(), entity);
    if (it != _children.end()) {
        remove_child(it - _children.begin());
    }
}


Entity *Scene::create_entity() {
    auto id = _entities.alloc();
    auto &entity = _entities.get<std::unique_ptr<EntityDerived>>(id);
    entity = std::make_unique<EntityDerived>();
    entity->scene = this;
    entity->id = id;

    return entity.get();
}

void Scene::destroy_entity(Entity *entity) {
    if (entity == nullptr) {
        return;
    }

    for (size_t i = 0; i < entity->child_count(); i++) {
        destroy_entity(entity->child(i));
    }

    auto parent = entity->parent();
    if (parent != nullptr) {
        parent->remove_child(entity);
    }
    auto derived = reinterpret_cast<EntityDerived *>(entity);
    assert(derived->scene == this);
    _entities.free(derived->id);
}

Scene::~Scene() = default;
} // namespace ars::scene
