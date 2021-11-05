#pragma once

#include "../Entity.h"
#include <ars/runtime/render/IContext.h>
#include <ars/runtime/render/IScene.h>

namespace ars::engine {
struct RenderSystem {
  public:
    explicit RenderSystem(render::IContext *context);

    void update();

    using Objects = SoA<Entity *, std::unique_ptr<render::IRenderObject>>;
    Objects objects{};

    using DirectionalLights =
        SoA<Entity *, std::unique_ptr<render::IDirectionalLight>>;
    DirectionalLights directional_lights{};

    using PointLights = SoA<Entity *, std::unique_ptr<render::IPointLight>>;
    PointLights point_lights{};

    std::unique_ptr<render::IScene> render_scene{};
};

class MeshRenderer : public IComponent {
    ARS_COMPONENT(MeshRenderer, IComponent);

  public:
    static void register_component();
    void init(Entity *entity) override;
    void destroy() override;

  private:
    Entity *_entity{};
    RenderSystem::Objects::Id _id{};
};

class PointLight : public IComponent {
    ARS_COMPONENT(PointLight, IComponent);

  public:
    static void register_component();
    void init(Entity *entity) override;
    void destroy() override;

  private:
    Entity *_entity{};
    RenderSystem::PointLights::Id _id{};
};

class DirectionalLight : public IComponent {
    ARS_COMPONENT(DirectionalLight, IComponent);

  public:
    static void register_component();
    void init(Entity *entity) override;
    void destroy() override;

  private:
    Entity *_entity{};
    RenderSystem::DirectionalLights::Id _id{};
};
} // namespace ars::engine