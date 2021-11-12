#pragma once

#include "../Entity.h"
#include <ars/runtime/render/IContext.h>
#include <ars/runtime/render/IScene.h>
#include <ars/runtime/render/res/Model.h>

namespace ars::engine {
struct RenderSystem {
  public:
    explicit RenderSystem(render::IContext *context);

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
    ARS_COMPONENT(MeshRenderer, IComponent);

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

    RenderSystem *_render_system{};
    RenderSystem::Objects::Id _id{};
};

class PointLight : public IComponent {
    ARS_COMPONENT(PointLight, IComponent);

  public:
    static void register_component();
    void init(Entity *entity) override;
    void destroy() override;
    [[nodiscard]] render::IPointLight *light() const;
    [[nodiscard]] Entity *entity() const;

  private:
    RenderSystem *_render_system{};
    RenderSystem::PointLights::Id _id{};
};

class DirectionalLight : public IComponent {
    ARS_COMPONENT(DirectionalLight, IComponent);

  public:
    static void register_component();
    void init(Entity *entity) override;
    void destroy() override;
    [[nodiscard]] render::IDirectionalLight *light() const;
    [[nodiscard]] Entity *entity() const;

  private:
    RenderSystem *_render_system{};
    RenderSystem::DirectionalLights::Id _id{};
};

// Load model as children of the parent entity
void load_model(Entity *parent, const render::Model &model);
} // namespace ars::engine