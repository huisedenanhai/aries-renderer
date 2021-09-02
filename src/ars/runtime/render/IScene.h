#pragma once

#include "../core/math/Transform.h"
#include <memory>

namespace ars::render {
class IMesh;
class ITexture;
class IMaterial;

class IScene;

class IRenderObject {
  public:
    virtual ~IRenderObject() = default;

    virtual math::AffineTransform<float> get_transform() = 0;
    virtual void
    set_transform(const math::AffineTransform<float> &transform) = 0;

    virtual IScene *get_scene() = 0;

    virtual IMesh *get_mesh() = 0;
    virtual void set_mesh(IMesh *mesh) = 0;

    virtual IMaterial *get_material() = 0;
    virtual void set_material(IMaterial *mesh) = 0;
};

class IDirectionalLight {
  public:
    virtual ~IDirectionalLight() = default;

    virtual math::AffineTransform<float> get_transform() = 0;
    virtual void
    set_transform(const math::AffineTransform<float> &transform) = 0;

    virtual IScene *get_scene() = 0;
};

// A rect to render to.
class IView {
  public:
    virtual ~IView() = default;

    virtual IScene *get_scene() = 0;

    // Render and update the texture
    virtual void render() = 0;
    virtual ITexture *get_color_texture() = 0;
};

class IScene {
  public:
    virtual ~IScene() = default;

    virtual std::unique_ptr<IRenderObject> create_renderer() = 0;
    virtual std::unique_ptr<IDirectionalLight> create_directional_light() = 0;
    virtual std::unique_ptr<IView> create_view() = 0;
};
} // namespace ars::render
