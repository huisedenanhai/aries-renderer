#include "Scene.h"
#include "Mesh.h"

namespace ars::render::vk {
std::unique_ptr<IRenderObject> Scene::create_render_object() {
    return std::make_unique<RenderObject>(this);
}

std::unique_ptr<IDirectionalLight> Scene::create_directional_light() {
    return std::make_unique<DirectionalLight>(this);
}

std::unique_ptr<IView> Scene::create_view() {
    return std::make_unique<View>(this);
}

IScene *View::get_scene() {
    return _scene;
}

void View::render() {
    // TODO
}

ITexture *View::get_color_texture() {
    // TODO
    return nullptr;
}

View::View(Scene *scene) : _scene(scene) {
    // TODO
}

math::XformTRS<float> DirectionalLight::get_xform() {
    return math::XformTRS<float>(get<glm::mat4>());
}

void DirectionalLight::set_xform(const math::XformTRS<float> &xform) {
    get<glm::mat4>() = xform.matrix();
}

IScene *DirectionalLight::get_scene() {
    return _scene;
}

DirectionalLight::DirectionalLight(Scene *scene) : _scene(scene) {
    _id = _scene->directional_lights.alloc();
}

DirectionalLight::~DirectionalLight() {
    _scene->directional_lights.free(_id);
}

math::XformTRS<float> RenderObject::get_xform() {
    return math::XformTRS<float>(get<glm::mat4>());
}

void RenderObject::set_xform(const math::XformTRS<float> &xform) {
    get<glm::mat4>() = xform.matrix();
}

IScene *RenderObject::get_scene() {
    return _scene;
}

IMesh *RenderObject::get_mesh() {
    return get<Mesh *>();
}

void RenderObject::set_mesh(IMesh *mesh) {
    get<Mesh *>() = upcast(mesh);
}

IMaterial *RenderObject::get_material() {
    return get<IMaterial *>();
}

void RenderObject::set_material(IMaterial *material) {
    get<IMaterial *>() = material;
}

RenderObject::RenderObject(Scene *scene) : _scene(scene) {
    _id = _scene->render_objects.alloc();
}

RenderObject::~RenderObject() {
    _scene->render_objects.free(_id);
}
} // namespace ars::render::vk
