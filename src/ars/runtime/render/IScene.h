#pragma once

#include "Common.h"
#include <array>
#include <ars/runtime/core/math/AABB.h>
#include <ars/runtime/core/math/Transform.h>
#include <memory>
#include <variant>
#include <vector>

// Aries uses right-hand coordinates, with Y points up, the same as the
// convention of GLTF
namespace ars::render {
class IEffect;
class IMesh;
class ITexture;
class IMaterial;
class IScene;

// Map scissor NDC region range [-1, 1]
// One can use this for projection jittering.
// Or use a none standard projection matrix that frames in the center of view.
glm::mat4 projection_scissor_correction(uint32_t x,
                                        uint32_t y,
                                        uint32_t width,
                                        uint32_t height,
                                        uint32_t image_width,
                                        uint32_t image_height);

class IRenderObject {
  public:
    virtual ~IRenderObject() = default;

    virtual math::XformTRS<float> xform() = 0;
    virtual void set_xform(const math::XformTRS<float> &xform) = 0;

    virtual IScene *scene() = 0;

    virtual std::shared_ptr<IMesh> mesh() = 0;
    virtual void set_mesh(std::shared_ptr<IMesh> mesh) = 0;

    // For RenderObjects managed by MeshRenderers of engine::Scene, user data is
    // set to Entity *. If the RenderObject is not managed, user data is
    // nullptr.
    virtual uint64_t user_data() = 0;
    virtual void set_user_data(uint64_t user_data) = 0;

    virtual std::shared_ptr<IMaterial> material() = 0;
    virtual void set_material(std::shared_ptr<IMaterial> material) = 0;
};

class IDirectionalLight {
  public:
    virtual ~IDirectionalLight() = default;

    // The light points at local -Z, i.e., the forward direction of the tranform
    virtual math::XformTRS<float> xform() = 0;
    virtual void set_xform(const math::XformTRS<float> &xform) = 0;
    // {1.0f, 1.0f, 1.0f} by default
    virtual glm::vec3 color() = 0;
    virtual void set_color(const glm::vec3 &color) = 0;
    // 1.0f by default
    virtual float intensity() = 0;
    virtual void set_intensity(float intensity) = 0;
    // The user data of managed lights is the Entity* in editor.
    virtual uint64_t user_data() = 0;
    virtual void set_user_data(uint64_t user_data) = 0;

    virtual bool is_sun() = 0;
    virtual void set_is_sun(bool is_sun) = 0;

    virtual IScene *scene() = 0;
};

class IPointLight {
  public:
    virtual ~IPointLight() = default;

    virtual math::XformTRS<float> xform() = 0;
    virtual void set_xform(const math::XformTRS<float> &xform) = 0;
    // {1.0f, 1.0f, 1.0f} by default
    virtual glm::vec3 color() = 0;
    virtual void set_color(const glm::vec3 &color) = 0;
    // 1.0f by default
    virtual float intensity() = 0;
    virtual void set_intensity(float intensity) = 0;
    // The user data of managed lights is the Entity* in editor.
    virtual uint64_t user_data() = 0;
    virtual void set_user_data(uint64_t user_data) = 0;

    virtual IScene *scene() = 0;
};

struct Frustum {
    // Planes for frustum culling.
    // A plane stores (n, c) for plane function dot(n, p) + c = 0.
    // n is normal, c is a const.
    // n points out from frustum. If dot(n, p) + c > 0, point p is certainly
    // not contained by the frustum.
    // n may be not normalized.
    glm::vec4 planes[6]{};

    bool culled(const math::AABB<float> &aabb) const;
};

Frustum transform_frustum(const glm::mat4 &mat, const Frustum &frustum);

struct Perspective {
    float y_fov = glm::radians(45.0f);
    float z_far = 100.0f; // z_far == 0 means infinity z_far
    float z_near = 0.1f;  // must > 0

    [[nodiscard]] glm::mat4 projection_matrix(float w_div_h) const;
    [[nodiscard]] Frustum frustum(float w_div_h) const;
};

struct Orthographic {
    float y_mag = 1.0f;
    float z_far = 100.0f; // must > 0
    float z_near = 0.1f;  // must > 0

    [[nodiscard]] glm::mat4 projection_matrix(float w_div_h) const;
    [[nodiscard]] Frustum frustum(float w_div_h) const;
};

struct CameraData : std::variant<Perspective, Orthographic> {
  private:
    using Base = std::variant<Perspective, Orthographic>;

  public:
    using Base::Base;

    [[nodiscard]] float z_far() const;
    [[nodiscard]] float z_near() const;

    // reversed-Z in range [0, 1], with -z_near mapped to 1.0 and -z_far mapped
    // to 0.0 output follows Vulkan spec with +Y axis point down
    [[nodiscard]] glm::mat4 projection_matrix(float w_div_h) const;
    [[nodiscard]] Frustum frustum(float w_div_h) const;
};

class IOverlay {
  public:
    virtual ~IOverlay() = default;

    // Outline color will be red by default
    virtual glm::vec4 outline_color(uint8_t group) = 0;
    virtual void set_outline_color(uint8_t group, const glm::vec4 &color) = 0;

    // The light gizmo will be rendered as a billboard in world space.
    // The light gizmo will affect object selection query.
    virtual void set_light_gizmo(const std::shared_ptr<ITexture> &texture,
                                 float width) = 0;

    // Draw functions
    // All draw requests will be cached and executed on the next call to
    // IView->render(), after that, all cached requests will be cleared.
    virtual void draw_outline(uint8_t group,
                              const math::XformTRS<float> &xform,
                              const std::shared_ptr<IMesh> &mesh) = 0;

    virtual void draw_line(const glm::vec3 &from,
                           const glm::vec3 &to,
                           const glm::vec4 &color) = 0;

    void draw_wire_box(const math::XformTRS<float> &xform,
                       const glm::vec3 &center,
                       const glm::vec3 &extent,
                       const glm::vec4 &color);
};

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

    // Return all object user data touched by the scissor region
    virtual std::vector<uint64_t> query_selection(uint32_t x,
                                                  uint32_t y,
                                                  uint32_t width,
                                                  uint32_t height) = 0;

    virtual IOverlay *overlay() = 0;

    // Not null
    virtual IEffect *effect() = 0;

    // Draw some debug info with ImGui
    virtual void debug_gui() {}

    [[nodiscard]] glm::mat4 view_matrix();
    [[nodiscard]] glm::mat4 projection_matrix();
    // Transform a quad with object space position (-0.5, -0.5) to (0.5, 0.5) to
    // HClip space
    [[nodiscard]] glm::mat4
    billboard_MV_matrix(const glm::vec3 &center_ws, float width, float height);
};

class IScene {
  public:
    virtual ~IScene() = default;

    virtual std::unique_ptr<IRenderObject> create_render_object() = 0;
    virtual std::unique_ptr<IDirectionalLight> create_directional_light() = 0;
    virtual std::unique_ptr<IPointLight> create_point_light() = 0;
    virtual std::unique_ptr<IView> create_view(const Extent2D &size) = 0;
};
} // namespace ars::render
