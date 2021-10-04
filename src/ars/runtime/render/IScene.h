#pragma once

#include "../core/math/Transform.h"
#include "Common.h"
#include <memory>
#include <variant>

namespace ars::render {
class IMesh;
class ITexture;
class IMaterial;

class IScene;

class IRenderObject {
  public:
    virtual ~IRenderObject() = default;

    virtual math::XformTRS<float> xform() = 0;
    virtual void set_xform(const math::XformTRS<float> &xform) = 0;

    virtual IScene *scene() = 0;

    virtual std::shared_ptr<IMesh> mesh() = 0;
    virtual void set_mesh(std::shared_ptr<IMesh> mesh) = 0;

    virtual std::shared_ptr<IMaterial> material() = 0;
    virtual void set_material(std::shared_ptr<IMaterial> material) = 0;
};

class IDirectionalLight {
  public:
    virtual ~IDirectionalLight() = default;

    virtual math::XformTRS<float> xform() = 0;
    virtual void set_xform(const math::XformTRS<float> &xform) = 0;

    virtual IScene *scene() = 0;
};

struct Perspective {
    float y_fov = glm::radians(45.0f);
    float z_far = 100.0f; // z_far == 0 means infinity z_far
    float z_near = 0.1f;  // must > 0
};

struct Orthographic {
    float y_mag = 1.0f;
    float z_far = 100.0f; // must > 0
    float z_near = 0.1f;  // must > 0
};

using CameraData = std::variant<Perspective, Orthographic>;

// A rect to render to.
class IView {
  public:
    virtual ~IView() = default;

    virtual IScene *scene() = 0;

    virtual math::XformTRS<float> xform() = 0;
    virtual void set_xform(const math::XformTRS<float> &xform) = 0;

    // by default a view holds a perspective camera with default values
    virtual CameraData camera() = 0;
    virtual void set_camera(const CameraData &camera) = 0;

    virtual Extent2D size() = 0;
    virtual void set_size(const Extent2D &size) = 0;

    // Render and update the texture
    virtual void render() = 0;
    virtual ITexture *get_color_texture() = 0;
};

class IScene {
  public:
    virtual ~IScene() = default;

    virtual std::unique_ptr<IRenderObject> create_render_object() = 0;
    virtual std::unique_ptr<IDirectionalLight> create_directional_light() = 0;
    virtual std::unique_ptr<IView> create_view(const Extent2D &size) = 0;
};
} // namespace ars::render
