#pragma once

#include "../IScene.h"

namespace ars::render::vk {
class View : public IView {
  public:
    IScene *get_scene() override;
    void render() override;
    ITexture *get_color_texture() override;
};

class DirectionalLight : public IDirectionalLight {
  public:
    math::AffineTransform<float> get_transform() override;
    void set_transform(const math::AffineTransform<float> &transform) override;
    IScene *get_scene() override;
};

class RenderObject : public IRenderObject {
  public:
    math::AffineTransform<float> get_transform() override;
    void set_transform(const math::AffineTransform<float> &transform) override;
    IScene *get_scene() override;
    IMesh *get_mesh() override;
    void set_mesh(IMesh *mesh) override;
    IMaterial *get_material() override;
    void set_material(IMaterial *mesh) override;
};

class Scene : public IScene {
  public:
    std::unique_ptr<IRenderObject> create_renderer() override;
    std::unique_ptr<IDirectionalLight> create_directional_light() override;
    std::unique_ptr<IView> create_view() override;
};
} // namespace ars::render
