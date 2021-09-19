#pragma once

#include "../IScene.h"
#include <ars/runtime/core/misc/SoA.h>

namespace ars::render::vk {
class Mesh;

class Scene : public IScene {
  public:
    std::unique_ptr<IRenderObject> create_render_object() override;
    std::unique_ptr<IDirectionalLight> create_directional_light() override;
    std::unique_ptr<IView> create_view() override;

    using RenderObjects = SoA<glm::mat4, Mesh *, IMaterial *>;
    RenderObjects render_objects{};

    using DirectionalLights = SoA<glm::mat4>;
    DirectionalLights directional_lights{};
};

class View : public IView {
  public:
    explicit View(Scene *scene);

    IScene *get_scene() override;
    void render() override;
    ITexture *get_color_texture() override;

  private:
    Scene *_scene = nullptr;
};

class DirectionalLight : public IDirectionalLight {
  public:
    explicit DirectionalLight(Scene *scene);
    ~DirectionalLight() override;

    math::XformTRS<float> get_xform() override;
    void set_xform(const math::XformTRS<float> &xform) override;
    IScene *get_scene() override;

  private:
    template <typename T> T &get() {
        return _scene->directional_lights.template get<T>(_id);
    }

    Scene *_scene = nullptr;
    Scene::DirectionalLights ::Id _id{};
};

class RenderObject : public IRenderObject {
  public:
    explicit RenderObject(Scene *scene);
    ~RenderObject() override;

    math::XformTRS<float> get_xform() override;
    void set_xform(const math::XformTRS<float> &xform) override;
    IScene *get_scene() override;
    IMesh *get_mesh() override;
    void set_mesh(IMesh *mesh) override;
    IMaterial *get_material() override;
    void set_material(IMaterial *material) override;

  private:
    template <typename T> T &get() {
        return _scene->render_objects.template get<T>(_id);
    }

    Scene *_scene = nullptr;
    Scene::RenderObjects::Id _id{};
};

} // namespace ars::render::vk