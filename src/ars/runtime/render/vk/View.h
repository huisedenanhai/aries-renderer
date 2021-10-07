#pragma once

#include "../IScene.h"
#include "RenderTarget.h"

namespace ars::render::vk {
class Scene;
class TextureAdapter;

class View : public IView {
  public:
    View(Scene *scene, const Extent2D &size);
    ~View() override;

    math::XformTRS<float> xform() override;
    void set_xform(const math::XformTRS<float> &xform) override;

    CameraData camera() override;
    void set_camera(const CameraData &camera) override;

    Extent2D size() override;
    void set_size(const Extent2D &size) override;

    IScene *scene() override;
    void render() override;

    ITexture *get_color_texture() override;

  private:
    void init_render_pass();
    void alloc_render_targets();
    void update_color_tex_adapter();
    TextureInfo color_tex_info() const;

    Scene *_scene = nullptr;
    math::XformTRS<float> _xform{};
    CameraData _camera = Perspective{};
    Extent2D _size{};

    std::unique_ptr<RenderTargetManager> _rt_manager{};
    RenderTargetId _color_rt_id{};

    std::unique_ptr<TextureAdapter> _color_tex_adapter{};

    VkRenderPass _render_pass = VK_NULL_HANDLE;
};
} // namespace ars::render::vk