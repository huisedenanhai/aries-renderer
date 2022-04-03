#include "Scene.h"
#include "Material.h"
#include "Mesh.h"
#include "Profiler.h"
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

CullingResult Scene::cull(const Frustum &frustum_ws) {
    CullingResult res{};

    res.scene = this;

    render_objects.for_each_id([&](auto id) {
        auto matrix = render_objects.get<glm::mat4>(id);
        auto mesh = render_objects.get<std::shared_ptr<Mesh>>(id);
        auto aabb_ws = math::transform_aabb(matrix, mesh->aabb());
        if (frustum_ws.culled(aabb_ws)) {
            return;
        }
        res.objects.push_back(id);
        if (res.objects.size() == 1) {
            res.visible_aabb_ws = aabb_ws;
        } else {
            res.visible_aabb_ws.extend_aabb(aabb_ws);
        }
    });

    return res;
}

math::AABB<float> Scene::loaded_aabb_ws() const {
    return _loaded_aabb_ws;
}

void Scene::update_loaded_aabb() {
    ARS_PROFILER_SAMPLE("Update Loaded AABB", 0xFF817135);
    auto obj_count = render_objects.size();
    if (obj_count == 0) {
        _loaded_aabb_ws = {};
        return;
    }
    auto matrix_arr = render_objects.get_array<glm::mat4>();
    auto mesh_arr = render_objects.get_array<std::shared_ptr<Mesh>>();
    _loaded_aabb_ws = math::transform_aabb(matrix_arr[0], mesh_arr[0]->aabb());
    for (int i = 1; i < obj_count; i++) {
        auto aabb_ws = math::transform_aabb(matrix_arr[i], mesh_arr[i]->aabb());
        _loaded_aabb_ws.extend_aabb(aabb_ws);
    }
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

std::vector<DrawRequest>
CullingResult::gather_draw_requests(RenderPassID pass_id) const {
    std::vector<DrawRequest> requests{};
    requests.reserve(objects.size());

    for (auto obj_id : objects) {
        auto &render_objects = scene->render_objects;
        auto matrix = render_objects.get<glm::mat4>(obj_id);
        auto mesh = render_objects.get<std::shared_ptr<Mesh>>(obj_id);
        auto material = render_objects.get<std::shared_ptr<Material>>(obj_id);

        DrawRequest req{};
        req.material = material->pass(pass_id);

        if (req.material.pipeline == nullptr) {
            continue;
        }

        req.M = matrix;
        req.mesh = mesh.get();
        requests.push_back(req);
    }

    return requests;
}
} // namespace ars::render::vk
