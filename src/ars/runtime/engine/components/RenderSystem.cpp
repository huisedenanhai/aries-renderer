#include "RenderSystem.h"
#include "../Engine.h"
#include <ars/runtime/core/Log.h>
#include <ars/runtime/core/Reflect.h>
#include <ars/runtime/render/IMaterial.h>
#include <ars/runtime/render/IMesh.h>

namespace ars::engine {
void RenderSystem::register_components() {
    MeshRenderer::register_component();
    PointLight::register_component();
    DirectionalLight::register_component();
    Camera::register_component();
}

void MeshRenderer::register_component() {
    rttr::registration::class_<Skin>("ars::engine::Skin")
        .property("name", &Skin::name)
        .property("joints", &Skin::joints)
        .property("inverse_binding_matrices", &Skin::inverse_binding_matrices);

    rttr::registration::class_<PrimitiveHandle>(
        "ars::engine::MeshRenderer::PrimitiveHandle")
        .property("mesh", &PrimitiveHandle::mesh)
        .property("material", &PrimitiveHandle::material);

    engine::register_component<MeshRenderer>("ars::engine::MeshRenderer")
        .property("primitives",
                  &MeshRenderer::primitive_handles,
                  &MeshRenderer::set_primitive_handles)
        .RTTR_MEMBER_PROPERTY(MeshRenderer, skin);
}

void MeshRenderer::init(Entity *entity) {
    _render_system = entity->scene()->render_system();
    auto &objs = _render_system->_objects;
    _id = objs.alloc();
    objs.get<Entity *>(_id) = entity;
    objs.get<MeshRenderer *>(_id) = this;
}

void MeshRenderer::destroy() {
    _render_system->_objects.free(_id);
}

std::vector<std::unique_ptr<render::IRenderObject>> &
MeshRenderer::primitives() {
    return _render_objects;
}

Entity *MeshRenderer::entity() const {
    return _render_system->_objects.get<Entity *>(_id);
}

size_t MeshRenderer::primitive_count() const {
    return _render_objects.size();
}

render::IRenderObject *MeshRenderer::primitive(size_t index) const {
    if (index >= primitive_count()) {
        ARS_LOG_ERROR(
            "Out of range primitive index {}, only {} primitives exists.",
            index,
            primitive_count());
        return nullptr;
    }
    return _render_objects[index].get();
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

std::vector<MeshRenderer::PrimitiveHandle>
MeshRenderer::primitive_handles() const {
    std::vector<PrimitiveHandle> handles{};
    handles.reserve(_render_objects.size());
    for (auto &p : _render_objects) {
        PrimitiveHandle h{};
        h.mesh = p->mesh();
        h.material = p->material();
        handles.push_back(h);
    }
    return handles;
}

void MeshRenderer::set_primitive_handles(std::vector<PrimitiveHandle> handles) {
    auto &prims = primitives();
    if (prims.size() > handles.size()) {
        prims.resize(handles.size());
    }
    while (prims.size() < handles.size()) {
        add_primitive();
    }
    assert(prims.size() == handles.size());
    for (int i = 0; i < handles.size(); i++) {
        prims[i]->set_mesh(
            std::dynamic_pointer_cast<render::IMesh>(handles[i].mesh));
        prims[i]->set_material(
            std::dynamic_pointer_cast<render::IMaterial>(handles[i].material));
    }
}

void MeshRenderer::update() {
    for (auto &obj : _render_objects) {
        obj->set_xform(entity()->cached_world_xform());
    }
    if (_skin != nullptr) {
        _skin->update();
    }
}

std::shared_ptr<Skin> MeshRenderer::skin() const {
    return _skin;
}

void MeshRenderer::set_skin(std::shared_ptr<Skin> skin) {
    _skin = skin;
    for (auto &obj : _render_objects) {
        obj->set_skin(skin ? skin->skin : nullptr);
    }
}

void PointLight::register_component() {
    engine::register_component<PointLight>("ars::engine::PointLight")
        .RTTR_MEMBER_PROPERTY(PointLight, color)(
            rttr::metadata(PropertyAttribute::Display, PropertyDisplay::Color))
        .RTTR_MEMBER_PROPERTY(PointLight, intensity);
}

void PointLight::init(Entity *entity) {
    _render_system = entity->scene()->render_system();
    auto &point_lights = _render_system->_point_lights;
    _id = point_lights.alloc();
    point_lights.get<Entity *>(_id) = entity;
    auto rd_light = _render_system->_render_scene->create_point_light();
    rd_light->set_user_data(reinterpret_cast<uint64_t>(entity));
    point_lights.get<std::unique_ptr<render::IPointLight>>(_id) =
        std::move(rd_light);
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

glm::vec3 PointLight::color() const {
    return light()->color();
}

void PointLight::set_color(glm::vec3 color) {
    light()->set_color(color);
}

float PointLight::intensity() const {
    return light()->intensity();
}

void PointLight::set_intensity(float intensity) {
    light()->set_intensity(intensity);
}

void DirectionalLight::register_component() {
    engine::register_component<DirectionalLight>(
        "ars::engine::DirectionalLight")
        .RTTR_MEMBER_PROPERTY(DirectionalLight, color)(
            rttr::metadata(PropertyAttribute::Display, PropertyDisplay::Color))
        .RTTR_MEMBER_PROPERTY(DirectionalLight, intensity)
        .RTTR_MEMBER_PROPERTY(DirectionalLight, is_sun);
}

void DirectionalLight::init(Entity *entity) {
    _render_system = entity->scene()->render_system();
    auto &lights = _render_system->_directional_lights;
    _id = lights.alloc();
    lights.get<Entity *>(_id) = entity;
    auto rd_light = _render_system->_render_scene->create_directional_light();
    rd_light->set_user_data(reinterpret_cast<uint64_t>(entity));
    lights.get<std::unique_ptr<render::IDirectionalLight>>(_id) =
        std::move(rd_light);
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

glm::vec3 DirectionalLight::color() const {
    return light()->color();
}

void DirectionalLight::set_color(glm::vec3 color) {
    light()->set_color(color);
}

float DirectionalLight::intensity() const {
    return light()->intensity();
}

void DirectionalLight::set_intensity(float intensity) {
    light()->set_intensity(intensity);
}

bool DirectionalLight::is_sun() const {
    return light()->is_sun();
}

void DirectionalLight::set_is_sun(bool is_sun) {
    light()->set_is_sun(is_sun);
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
    auto obj_count = _objects.size();
    auto obj_arr = _objects.get_array<MeshRenderer *>();
    for (int i = 0; i < obj_count; i++) {
        obj_arr[i]->update();
    }

    update_xform(_objects, [](auto &&ts, auto &&xform) {});
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
               std::vector<Entity *> &entities,
               render::Model::Index node) {
    if (entities[node] == nullptr) {
        entities[node] = parent->scene()->create_entity();
    }
    auto entity = entities[node];

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

    if (n.camera.has_value()) {
        auto c = model.cameras[n.camera.value()];
        auto comp = entity->add_component<Camera>();
        comp->set_data(c.data);
    }

    for (auto child : n.children) {
        load_node(entity, model, entities, child);
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
    std::vector<Entity *> entities{};
    entities.resize(model.nodes.size());
    for (auto n : scene.nodes) {
        load_node(parent, model, entities, n);
    }
    // Set up links
    for (int i = 0; i < entities.size(); i++) {
        auto entity = entities[i];
        auto &n = model.nodes[i];
        if (entity == nullptr) {
            continue;
        }
        if (n.mesh.has_value() && n.skin.has_value()) {
            auto &s = model.skins[n.skin.value()];
            auto skin = std::make_shared<Skin>();
            skin->name = s.name;
            skin->inverse_binding_matrices = s.inverse_binding_matrices;
            skin->joints.reserve(s.joints.size());
            for (auto j : s.joints) {
                skin->joints.push_back(entities[j]);
            }

            render::SkinInfo info{};
            info.joint_count = s.joints.size();
            skin->skin = engine::render_context()->create_skin(info);

            entity->component<MeshRenderer>()->set_skin(skin);
        }
    }
}

void Camera::register_component() {
    engine::register_component<Camera>("ars::engine::Camera");
}

void Camera::set_data(render::CameraData data) {
    _data = data;
}

render::CameraData Camera::data() const {
    return _data;
}

void Skin::update() {
    assert(skin != nullptr);
    assert(joints.size() == inverse_binding_matrices.size());
    assert(joints.size() == skin->joint_count());

    auto joint_count = joints.size();
    std::vector<glm::mat4> joint_mats{};
    joint_mats.resize(joint_count);
    for (int i = 0; i < joint_count; i++) {
        joint_mats[i] = joints[i]->cached_world_xform().matrix() *
                        inverse_binding_matrices[i];
    }

    skin->set_joints(joint_mats.data(), 0, joint_count);
}
} // namespace ars::engine
