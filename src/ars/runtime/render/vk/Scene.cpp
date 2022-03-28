#include "Scene.h"
#include "Material.h"
#include "Mesh.h"
#include "View.h"
#include "features/Drawer.h"

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

std::vector<DrawRequest> Scene::gather_draw_requests(RenderPassID pass_id) {
    // TODO culling
    auto count = render_objects.size();
    std::vector<DrawRequest> requests{};
    requests.reserve(count);

    auto xform_arr = render_objects.get_array<glm::mat4>();
    auto mesh_arr = render_objects.get_array<std::shared_ptr<Mesh>>();
    auto mat_arr = render_objects.get_array<std::shared_ptr<Material>>();

    for (int i = 0; i < count; i++) {
        DrawRequest req{};
        req.material = mat_arr[i]->pass(pass_id);

        if (req.material.pipeline == nullptr) {
            continue;
        }

        req.M = xform_arr[i];
        req.mesh = mesh_arr[i].get();
        requests.push_back(req);
    }

    return requests;
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
    _scene->sun_id = {};
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

uint64_t DirectionalLight::user_data() {
    return get<UserData>().value;
}

void DirectionalLight::set_user_data(uint64_t user_data) {
    get<UserData>().value = user_data;
}

bool DirectionalLight::is_sun() {
    return _id == _scene->sun_id;
}

void DirectionalLight::set_is_sun(bool is_sun) {
    if (is_sun) {
        _scene->sun_id = _id;
    } else if (_scene->sun_id == _id) {
        _scene->sun_id = {};
    }
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
    return get<std::shared_ptr<Material>>();
}

void RenderObject::set_material(std::shared_ptr<IMaterial> material) {
    get<std::shared_ptr<Material>>() = upcast(material);
}

RenderObject::RenderObject(Scene *scene) : _scene(scene) {
    _id = _scene->render_objects.alloc();
}

RenderObject::~RenderObject() {
    _scene->render_objects.free(_id);
}

uint64_t RenderObject::user_data() {
    return get<UserData>().value;
}

void RenderObject::set_user_data(uint64_t user_data) {
    get<UserData>().value = user_data;
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

uint64_t PointLight::user_data() {
    return get<UserData>().value;
}

void PointLight::set_user_data(uint64_t user_data) {
    get<UserData>().value = user_data;
}
} // namespace ars::render::vk
