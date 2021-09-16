#pragma once

#include "../IScene.h"
#include <ars/runtime/core/misc/SoA.h>

namespace ars::render::vk {
class View : public IView {
  public:
    IScene *get_scene() override;
    void render() override;
    ITexture *get_color_texture() override;
};

class DirectionalLight : public IDirectionalLight {
  public:
    math::XformTRS<float> get_xform() override;
    void set_xform(const math::XformTRS<float> &xform) override;
    IScene *get_scene() override;
};

class RenderObject : public IRenderObject {
  public:
    math::XformTRS<float> get_xform() override;
    void set_xform(const math::XformTRS<float> &xform) override;
    IScene *get_scene() override;
    IMesh *get_mesh() override;
    void set_mesh(IMesh *mesh) override;
    IMaterial *get_material() override;
    void set_material(IMaterial *mesh) override;
};

class Scene : public IScene {
  public:
    std::unique_ptr<IRenderObject> create_render_object() override;
    std::unique_ptr<IDirectionalLight> create_directional_light() override;
    std::unique_ptr<IView> create_view() override;

  private:
    SoA<glm::mat4> _render_objects{};
};
} // namespace ars::render::vk
