#pragma once

#include "../IScene.h"
#include "RenderTarget.h"

namespace ars::render::vk {
class Scene;
class TextureAdapter;
class GraphicsPipeline;
class Renderer;
class OverlayRenderer;
class RenderPass;
class Drawer;

enum NamedRT {
    NamedRT_GBuffer0, // for base color
    NamedRT_GBuffer1, // for normal
    NamedRT_GBuffer2, // for material
    NamedRT_GBuffer3, // for emission
    NamedRT_Depth,
    NamedRT_FinalColor0,
    NamedRT_FinalColor1,

    NamedRT_Count
};

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

    std::vector<uint64_t> query_selection(uint32_t x,
                                          uint32_t y,
                                          uint32_t width,
                                          uint32_t height) override;
    IOverlay *overlay() override;
    void set_environment_radiance(const glm::vec3 &radiance) override;
    glm::vec3 environment_radiance() override;
    void set_environment_cube_map(
        const std::shared_ptr<ITexture> &cube_map) override;
    std::shared_ptr<ITexture> environment_cube_map() override;

    OverlayRenderer *vk_overlay() const;

    [[nodiscard]] Context *context() const;
    [[nodiscard]] Scene *vk_scene() const;

    [[nodiscard]] Handle<Texture> render_target(NamedRT name) const;
    [[nodiscard]] RenderTargetManager *rt_manager() const;
    [[nodiscard]] RenderTargetId rt_id(NamedRT name) const;
    [[nodiscard]] RenderTargetInfo rt_info(NamedRT name) const;
    [[nodiscard]] std::unique_ptr<RenderPass> create_single_pass_render_pass(
        NamedRT *colors, uint32_t color_count, NamedRT depth_stencil) const;

    [[nodiscard]] Drawer *drawer() const;

  private:
    void alloc_render_targets();
    void update_color_tex_adapter(NamedRT rt);
    [[nodiscard]] TextureInfo color_tex_info() const;

    Scene *_scene = nullptr;
    math::XformTRS<float> _xform{};
    CameraData _camera = Perspective{};
    Extent2D _size{};

    std::unique_ptr<RenderTargetManager> _rt_manager{};
    RenderTargetId _rt_ids[NamedRT_Count]{};

    std::unique_ptr<TextureAdapter> _color_tex_adapter{};

    std::unique_ptr<Renderer> _renderer{};
    std::unique_ptr<OverlayRenderer> _overlay_renderer{};
    std::unique_ptr<Drawer> _drawer{};
    glm::vec3 _env_radiance = glm::vec3(0.1f);
    std::shared_ptr<ITexture> _env_cube_map{};
};
} // namespace ars::render::vk