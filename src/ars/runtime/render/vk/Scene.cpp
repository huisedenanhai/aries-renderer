#include "Scene.h"
#include "Mesh.h"
#include "View.h"

namespace ars::render::vk {
std::unique_ptr<IRenderObject> Scene::create_render_object() {
    return std::make_unique<RenderObject>(this);
}

std::unique_ptr<IDirectionalLight> Scene::create_directional_light() {
    return std::make_unique<DirectionalLight>(this);
}

std::unique_ptr<IView> Scene::create_view(const Extent2D &size) {
    return std::make_unique<View>(this, size);
}

Scene::Scene(Context *context) : _context(context) {}

Context *Scene::context() const {
    return _context;
}

IScene *View::scene() {
    return _scene;
}

math::XformTRS<float> DirectionalLight::xform() {
    return math::XformTRS<float>(get<glm::mat4>());
}

void DirectionalLight::set_xform(const math::XformTRS<float> &xform) {
    get<glm::mat4>() = xform.matrix();
}

IScene *DirectionalLight::scene() {
    return _scene;
}

DirectionalLight::DirectionalLight(Scene *scene) : _scene(scene) {
    _id = _scene->directional_lights.alloc();
}

DirectionalLight::~DirectionalLight() {
    _scene->directional_lights.free(_id);
}

math::XformTRS<float> RenderObject::xform() {
    return math::XformTRS<float>(get<glm::mat4>());
}

void RenderObject::set_xform(const math::XformTRS<float> &xform) {
    get<glm::mat4>() = xform.matrix();
}

IScene *RenderObject::scene() {
    return _scene;
}

std::shared_ptr<IMesh> RenderObject::mesh() {
    return get<std::shared_ptr<Mesh>>();
}

void RenderObject::set_mesh(std::shared_ptr<IMesh> mesh) {
    get<std::shared_ptr<Mesh>>() = upcast(mesh);
}

std::shared_ptr<IMaterial> RenderObject::material() {
    return get<std::shared_ptr<IMaterial>>();
}

void RenderObject::set_material(std::shared_ptr<IMaterial> material) {
    get<std::shared_ptr<IMaterial>>() = std::move(material);
}

RenderObject::RenderObject(Scene *scene) : _scene(scene) {
    _id = _scene->render_objects.alloc();
}

RenderObject::~RenderObject() {
    _scene->render_objects.free(_id);
}
} // namespace ars::render::vk
