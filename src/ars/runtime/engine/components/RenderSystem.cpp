#include "RenderSystem.h"

namespace ars::engine {
void MeshRenderer::register_component() {
    engine::register_component<MeshRenderer>("ars::engine::MeshRenderer");
}

void MeshRenderer::init(Entity *entity) {
    _entity = entity;
    auto rd_sys = entity->scene()->render_system();
    auto &objs = rd_sys->objects;
    _id = objs.alloc();
    objs.get<Entity *>(_id) = entity;
    objs.get<std::unique_ptr<render::IRenderObject>>(_id) =
        rd_sys->render_scene->create_render_object();
}

void MeshRenderer::destroy() {
    _entity->scene()->render_system()->objects.free(_id);
}

void PointLight::register_component() {
    engine::register_component<PointLight>("ars::engine::PointLight");
}

void PointLight::init(Entity *entity) {
    _entity = entity;
    auto rd_sys = entity->scene()->render_system();
    auto &point_lights = rd_sys->point_lights;
    _id = point_lights.alloc();
    point_lights.get<Entity *>(_id) = entity;
    point_lights.get<std::unique_ptr<render::IPointLight>>(_id) =
        rd_sys->render_scene->create_point_light();
}

void PointLight::destroy() {
    _entity->scene()->render_system()->point_lights.free(_id);
}

void DirectionalLight::register_component() {
    engine::register_component<DirectionalLight>(
        "ars::engine::DirectionalLight");
}

void DirectionalLight::init(Entity *entity) {
    _entity = entity;
    auto rd_sys = entity->scene()->render_system();
    auto &lights = rd_sys->directional_lights;
    _id = lights.alloc();
    lights.get<Entity *>(_id) = entity;
    lights.get<std::unique_ptr<render::IDirectionalLight>>(_id) =
        rd_sys->render_scene->create_directional_light();
}

void DirectionalLight::destroy() {
    _entity->scene()->render_system()->directional_lights.free(_id);
}

RenderSystem::RenderSystem(render::IContext *context) {
    render_scene = context->create_scene();
}

template <typename T>
void update_xform(SoA<Entity *, std::unique_ptr<T>> &soa) {
    auto count = soa.size();
    auto entities = soa.template get_array<Entity *>();
    auto targets = soa.template get_array<std::unique_ptr<T>>();
    for (int i = 0; i < count; i++) {
        targets[i]->set_xform(entities[i]->cached_world_xform());
    }
}

void RenderSystem::update() {
    update_xform(objects);
    update_xform(directional_lights);
    update_xform(point_lights);
}
} // namespace ars::engine