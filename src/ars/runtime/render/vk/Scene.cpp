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

std::unique_ptr<IPointLight> Scene::create_point_light() {
    return std::make_unique<PointLight>(this);
}

Scene::Scene(Context *context) : _context(context) {}

Context *Scene::context() const {
    return _context;
}

IScene *View::scene() {
    return _scene;
}

math::XformTRS<float> DirectionalLight::xform() {
    return get<math::XformTRS<float>>();
}

void DirectionalLight::set_xform(const math::XformTRS<float> &xform) {
    get<math::XformTRS<float>>() = xform;
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

glm::vec3 DirectionalLight::color() {
    return get<Light>().color;
}

void DirectionalLight::set_color(const glm::vec3 &color) {
    get<Light>().color = color;
}

float DirectionalLight::intensity() {
    return get<Light>().intensity;
}

void DirectionalLight::set_intensity(float intensity) {
    get<Light>().intensity = intensity;
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

uint64_t RenderObject::user_data() {
    return get<RenderObjectUserData>().value;
}

void RenderObject::set_user_data(uint64_t user_data) {
    get<RenderObjectUserData>().value = user_data;
}

PointLight::PointLight(Scene *scene) : _scene(scene) {
    _id = _scene->point_lights.alloc();
}

PointLight::~PointLight() {
    _scene->point_lights.free(_id);
}

IScene *PointLight::scene() {
    return _scene;
}

void PointLight::set_xform(const math::XformTRS<float> &xform) {
    get<math::XformTRS<float>>() = xform;
}

math::XformTRS<float> PointLight::xform() {
    return get<math::XformTRS<float>>();
}

glm::vec3 PointLight::color() {
    return get<Light>().color;
}

void PointLight::set_color(const glm::vec3 &color) {
    get<Light>().color = color;
}

float PointLight::intensity() {
    return get<Light>().intensity;
}

void PointLight::set_intensity(float intensity) {
    get<Light>().intensity = intensity;
}
} // namespace ars::render::vk
