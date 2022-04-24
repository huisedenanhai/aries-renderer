#pragma once

#include "../IScene.h"
#include "features/Renderer.h"
#include "features/Shadow.h"
#include <ars/runtime/core/misc/SoA.h>

namespace ars::render::vk {
class Mesh;
class Skin;
class Material;
class Context;
struct DrawRequest;

struct Light {
    glm::vec3 color{1.0f, 1.0f, 1.0f};
    float intensity = 1.0f;
};

struct UserData {
    uint64_t value = 0;
};

struct CullingResult;

class Scene : public IScene {
  public:
    explicit Scene(Context *context);

    std::unique_ptr<IRenderObject> create_render_object() override;
    std::unique_ptr<IDirectionalLight> create_directional_light() override;
    std::unique_ptr<IPointLight> create_point_light() override;
    std::unique_ptr<IView> create_view(const Extent2D &size) override;

    [[nodiscard]] Context *context() const;

    CullingResult cull(const math::XformTRS<float> &xform,
                       const Frustum &frustum_local);

    void update_loaded_aabb();
    math::AABB<float> loaded_aabb_ws() const;

    using RenderObjects = SoA<glm::mat4,
                              std::shared_ptr<Mesh>,
                              std::shared_ptr<Material>,
                              std::shared_ptr<Skin>,
                              UserData>;
    RenderObjects render_objects{};

    using DirectionalLights =
        SoA<math::XformTRS<float>, Light, UserData, std::unique_ptr<ShadowMap>>;
    DirectionalLights directional_lights{};

    using PointLights = SoA<math::XformTRS<float>, Light, UserData>;
    PointLights point_lights{};

    DirectionalLights::Id sun_id{};

    std::optional<DrawRequest>
    get_draw_request(RenderPassID pass_id,
                     RenderObjects::Id rd_obj,
                     Material *override_material = nullptr);

  private:
    CullingResult cull(const Frustum &frustum_ws);

    Context *_context = nullptr;
    math::AABB<float> _loaded_aabb_ws{};
};

struct CullingResult {
    Scene *scene = nullptr;
    std::vector<Scene::RenderObjects::Id> objects{};
    math::AABB<float> visible_aabb_ws{};

    std::vector<DrawRequest> gather_draw_requests(RenderPassID pass_id) const;
};

class DirectionalLight : public IDirectionalLight {
  public:
    explicit DirectionalLight(Scene *scene);
    ~DirectionalLight() override;

    math::XformTRS<float> xform() override;
    void set_xform(const math::XformTRS<float> &xform) override;
    glm::vec3 color() override;
    void set_color(const glm::vec3 &color) override;
    float intensity() override;
    void set_intensity(float intensity) override;
    IScene *scene() override;
    uint64_t user_data() override;
    void set_user_data(uint64_t user_data) override;
    bool is_sun() override;
    void set_is_sun(bool is_sun) override;

  private:
    template <typename T> T &get() {
        return _scene->directional_lights.template get<T>(_id);
    }

    Scene *_scene = nullptr;
    Scene::DirectionalLights::Id _id{};
};

class PointLight : public IPointLight {
  public:
    explicit PointLight(Scene *scene);
    ~PointLight() override;

    math::XformTRS<float> xform() override;
    void set_xform(const math::XformTRS<float> &xform) override;
    glm::vec3 color() override;
    void set_color(const glm::vec3 &color) override;
    float intensity() override;
    void set_intensity(float intensity) override;
    uint64_t user_data() override;
    void set_user_data(uint64_t user_data) override;
    IScene *scene() override;

  private:
    template <typename T> T &get() {
        return _scene->point_lights.template get<T>(_id);
    }

    Scene *_scene = nullptr;
    Scene::PointLights::Id _id{};
};

class RenderObject : public IRenderObject {
  public:
    explicit RenderObject(Scene *scene);
    ~RenderObject() override;

    math::XformTRS<float> xform() override;
    void set_xform(const math::XformTRS<float> &xform) override;
    IScene *scene() override;
    std::shared_ptr<IMesh> mesh() override;
    void set_mesh(std::shared_ptr<IMesh> mesh) override;
    std::shared_ptr<ISkin> skin() override;
    void set_skin(std::shared_ptr<ISkin> skin) override;
    std::shared_ptr<IMaterial> material() override;
    void set_material(std::shared_ptr<IMaterial> material) override;
    uint64_t user_data() override;
    void set_user_data(uint64_t user_data) override;
    Scene::RenderObjects::Id id() const;

  private:
    template <typename T> T &get() {
        return _scene->render_objects.template get<T>(_id);
    }

    Scene *_scene = nullptr;
    Scene::RenderObjects::Id _id{};
};

RenderObject *upcast(IRenderObject *rd_obj);

} // namespace ars::render::vk
