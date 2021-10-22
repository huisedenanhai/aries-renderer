#include "Entity.h"
#include "../Log.h"
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

Entity::Entity(Scene *scene) : _scene(scene) {
    assert(scene != nullptr);
}

Scene *Entity::scene() const {
    return _scene;
}

Entity *Scene::create_entity() {
    auto id = _entities.alloc();
    auto &entity = _entities.get<std::unique_ptr<EntityDerived>>(id);
    entity = std::make_unique<EntityDerived>(this);
    entity->id = id;
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
    auto derived = reinterpret_cast<EntityDerived *>(entity);
    _entities.free(derived->id);
}

Entity *Scene::root() const {
    return _root;
}

Scene::Scene() {
    _root = create_entity();
    _root->set_name("ROOT");
}

Scene::~Scene() = default;

} // namespace ars::scene
