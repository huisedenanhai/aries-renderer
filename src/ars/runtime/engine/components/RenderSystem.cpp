#include "RenderSystem.h"
#include <ars/runtime/core/Log.h>

namespace ars::engine {
void MeshRenderer::register_component() {
    engine::register_component<MeshRenderer>("ars::engine::MeshRenderer");
}

void MeshRenderer::init(Entity *entity) {
    _render_system = entity->scene()->render_system();
    auto &objs = _render_system->_objects;
    _id = objs.alloc();
    objs.get<Entity *>(_id) = entity;
}

void MeshRenderer::destroy() {
    _render_system->_objects.free(_id);
}

std::vector<std::unique_ptr<render::IRenderObject>> &
MeshRenderer::primitives() const {
    return _render_system->_objects
        .get<std::vector<std::unique_ptr<render::IRenderObject>>>(_id);
}

Entity *MeshRenderer::entity() const {
    return _render_system->_objects.get<Entity *>(_id);
}

size_t MeshRenderer::primitive_count() const {
    return primitives().size();
}

render::IRenderObject *MeshRenderer::primitive(size_t index) const {
    if (index >= primitive_count()) {
        ARS_LOG_ERROR(
            "Out of range primitive index {}, only {} primitives exists.",
            index,
            primitive_count());
        return nullptr;
    }
    return primitives()[index].get();
}

render::IRenderObject *MeshRenderer::add_primitive() {
    static_assert(sizeof(void *) <= sizeof(uint64_t));
    auto rd_obj = _render_system->render_scene()->create_render_object();
    rd_obj->set_user_data(reinterpret_cast<uint64_t>(entity()));
    primitives().emplace_back(std::move(rd_obj));
    return primitives().back().get();
}

void MeshRenderer::remove_primitive(size_t index) {
    auto &prims = primitives();
    if (index >= prims.size()) {
        return;
    }
    prims.erase(std::next(prims.begin(), static_cast<int>(index)));
}

void PointLight::register_component() {
    engine::register_component<PointLight>("ars::engine::PointLight");
}

void PointLight::init(Entity *entity) {
    _render_system = entity->scene()->render_system();
    auto &point_lights = _render_system->_point_lights;
    _id = point_lights.alloc();
    point_lights.get<Entity *>(_id) = entity;
    point_lights.get<std::unique_ptr<render::IPointLight>>(_id) =
        _render_system->_render_scene->create_point_light();
}

void PointLight::destroy() {
    _render_system->_point_lights.free(_id);
}

render::IPointLight *PointLight::light() const {
    return _render_system->_point_lights
        .get<std::unique_ptr<render::IPointLight>>(_id)
        .get();
}

Entity *PointLight::entity() const {
    return _render_system->_point_lights.get<Entity *>(_id);
}

void DirectionalLight::register_component() {
    engine::register_component<DirectionalLight>(
        "ars::engine::DirectionalLight");
}

void DirectionalLight::init(Entity *entity) {
    _render_system = entity->scene()->render_system();
    auto &lights = _render_system->_directional_lights;
    _id = lights.alloc();
    lights.get<Entity *>(_id) = entity;
    lights.get<std::unique_ptr<render::IDirectionalLight>>(_id) =
        _render_system->_render_scene->create_directional_light();
}

void DirectionalLight::destroy() {
    _render_system->_directional_lights.free(_id);
}

render::IDirectionalLight *DirectionalLight::light() const {
    return _render_system->_directional_lights
        .get<std::unique_ptr<render::IDirectionalLight>>(_id)
        .get();
}

Entity *DirectionalLight::entity() const {
    return _render_system->_directional_lights.get<Entity *>(_id);
}

RenderSystem::RenderSystem(render::IContext *context) {
    _render_scene = context->create_scene();
}

template <typename T, typename Func>
void update_xform(SoA<Entity *, T> &soa, Func &&updater) {
    auto count = soa.size();
    auto entities = soa.template get_array<Entity *>();
    auto targets = soa.template get_array<T>();
    for (int i = 0; i < count; i++) {
        updater(targets[i], entities[i]->cached_world_xform());
    }
}

void RenderSystem::update() {
    update_xform(_objects, [](auto &&ts, auto &&xform) {
        for (auto &t : ts) {
            t->set_xform(xform);
        }
    });
    update_xform(_directional_lights,
                 [](auto &&t, auto &&xform) { t->set_xform(xform); });
    update_xform(_point_lights,
                 [](auto &&t, auto &&xform) { t->set_xform(xform); });
}

render::IScene *RenderSystem::render_scene() const {
    return _render_scene.get();
}

namespace {
void load_node(Entity *parent,
               const render::Model &model,
               render::Model::Index node) {
    auto entity = parent->scene()->create_entity();
    auto &n = model.nodes[node];
    entity->set_name(n.name);
    entity->set_parent(parent);
    entity->set_local_xform(n.local_to_parent);

    if (n.mesh.has_value()) {
        auto &m = model.meshes[n.mesh.value()];
        auto comp = entity->add_component<MeshRenderer>();
        for (auto &p : m.primitives) {
            auto rd_obj = comp->add_primitive();
            rd_obj->set_mesh(p.mesh);

            if (p.material.has_value()) {
                auto &mat = model.materials[p.material.value()];
                rd_obj->set_material(mat.material);
            }
        }
    }

    if (n.light.has_value()) {
        auto &l = model.lights[n.light.value()];
        auto set_up_light = [&](auto &&light,
                                const render::Model::Light &data) {
            light->set_color(data.color);
            light->set_intensity(data.intensity);
        };
        if (l.type == render::Model::Directional) {
            auto comp = entity->add_component<DirectionalLight>();
            set_up_light(comp->light(), l);
        }
        if (l.type == render::Model::Point) {
            auto comp = entity->add_component<PointLight>();
            set_up_light(comp->light(), l);
        }
    }

    for (auto child : n.children) {
        load_node(entity, model, child);
    }
}
} // namespace

void load_model(Entity *parent, const render::Model &model) {
    if (parent == nullptr) {
        ARS_LOG_ERROR("Can not load model to the null entity");
        return;
    }

    if (model.scenes.empty()) {
        return;
    }
    auto &scene = model.scenes[model.default_scene.value_or(0)];
    for (auto n : scene.nodes) {
        load_node(parent, model, n);
    }
}
} // namespace ars::engine