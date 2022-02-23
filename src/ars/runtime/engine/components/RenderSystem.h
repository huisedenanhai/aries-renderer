#pragma once

#include "../Entity.h"
#include <ars/runtime/core/Reflect.h>
#include <ars/runtime/render/IContext.h>
#include <ars/runtime/render/IScene.h>
#include <ars/runtime/render/res/Model.h>

namespace ars::engine {
struct RenderSystem {
  public:
    explicit RenderSystem(render::IContext *context);

    static void register_components();

    void update();
    [[nodiscard]] render::IScene *render_scene() const;

  private:
    friend class MeshRenderer;
    friend class PointLight;
    friend class DirectionalLight;

    using Objects =
        SoA<Entity *, std::vector<std::unique_ptr<render::IRenderObject>>>;
    Objects _objects{};

    using DirectionalLights =
        SoA<Entity *, std::unique_ptr<render::IDirectionalLight>>;
    DirectionalLights _directional_lights{};

    using PointLights = SoA<Entity *, std::unique_ptr<render::IPointLight>>;
    PointLights _point_lights{};

    std::unique_ptr<render::IScene> _render_scene{};
};

class MeshRenderer : public IComponent {
    RTTR_DERIVE(IComponent);

  public:
    static void register_component();
    void init(Entity *entity) override;
    void destroy() override;
    // User data of IRenderObject is set to the pointer of entity.
    [[nodiscard]] size_t primitive_count() const;
    [[nodiscard]] render::IRenderObject *primitive(size_t index) const;
    render::IRenderObject *add_primitive();
    void remove_primitive(size_t index);
    [[nodiscard]] Entity *entity() const;

  private:
    [[nodiscard]] std::vector<std::unique_ptr<render::IRenderObject>> &
    primitives() const;

    struct PrimitiveHandle {
        std::shared_ptr<IRes> mesh;
        std::shared_ptr<IRes> material;
    };

    [[nodiscard]] std::vector<PrimitiveHandle> primitive_handles() const;
    void set_primitive_handles(std::vector<PrimitiveHandle> handles);

    RenderSystem *_render_system{};
    RenderSystem::Objects::Id _id{};
};

class PointLight : public IComponent {
    RTTR_DERIVE(IComponent);

  public:
    static void register_component();
    void init(Entity *entity) override;
    void destroy() override;
    [[nodiscard]] render::IPointLight *light() const;
    [[nodiscard]] Entity *entity() const;
    [[nodiscard]] glm::vec3 color() const;
    void set_color(glm::vec3 color);
    [[nodiscard]] float intensity() const;
    void set_intensity(float intensity);

  private:
    RenderSystem *_render_system{};
    RenderSystem::PointLights::Id _id{};
};

class DirectionalLight : public IComponent {
    RTTR_DERIVE(IComponent);

  public:
    static void register_component();
    void init(Entity *entity) override;
    void destroy() override;
    [[nodiscard]] render::IDirectionalLight *light() const;
    [[nodiscard]] Entity *entity() const;
    [[nodiscard]] glm::vec3 color() const;
    void set_color(glm::vec3 color);
    [[nodiscard]] float intensity() const;
    void set_intensity(float intensity);
    [[nodiscard]] bool is_sun() const;
    void set_is_sun(bool is_sun);

  private:
    RenderSystem *_render_system{};
    RenderSystem::DirectionalLights::Id _id{};
};

class Camera : public IComponent {
    RTTR_DERIVE(IComponent);

  public:
    static void register_component();

    void set_data(render::CameraData data);
    [[nodiscard]] render::CameraData data() const;

  private:
    render::CameraData _data{};
};

// Load model as children of the parent entity
void load_model(Entity *parent, const render::Model &model);
} // namespace ars::engine
