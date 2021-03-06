#pragma once

#include "../IScene.h"
#include "RenderTarget.h"

namespace ars::render::vk {
class Buffer;
class Effect;
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
    // Anti-aliased color without further post-processing
    NamedRT_LinearColor,
    NamedRT_LinearColorHistory,
    // Ping-pong buffer for post-processing
    NamedRT_PostProcessing0,
    NamedRT_PostProcessing1,

    // Stores max value in r channel, min value WITHOUT background in g channel
    NamedRT_HiZBuffer,
    NamedRT_Reflection,
    // Auto swapped with NamedRT_Reflection on frame end
    NamedRT_ReflectionHistory,
    NamedRT_SSGI,
    // Auto swapped with NamedRT_Reflection on frame end
    NamedRT_SSGIHistory,

    NamedRT_Count
};

constexpr VkFormat RT_FORMAT_DEFAULT_HDR = VK_FORMAT_B10G11R11_UFLOAT_PACK32;
constexpr VkFormat RT_FORMAT_GBUFFER0 = VK_FORMAT_R8G8B8A8_SRGB;
constexpr VkFormat RT_FORMAT_GBUFFER1 = VK_FORMAT_A2R10G10B10_UNORM_PACK32;
constexpr VkFormat RT_FORMAT_GBUFFER2 = VK_FORMAT_R8G8B8A8_UNORM;
constexpr VkFormat RT_FORMAT_GBUFFER3 = RT_FORMAT_DEFAULT_HDR;
constexpr VkFormat RT_FORMAT_DEPTH = VK_FORMAT_D32_SFLOAT;

struct ViewTransform {
    glm::mat4 V;
    glm::mat4 P;
    glm::mat4 I_V;
    glm::mat4 I_P;
    glm::mat4 prev_V;
    glm::mat4 prev_P;
    glm::mat4 prev_IV;
    glm::mat4 prev_IP;
    glm::mat4 reproject_IV_VP;
    float z_near;
    float z_far;

    static ViewTransform from_V_P(const glm::mat4 &V, const glm::mat4 &P);
};

struct ViewData {
    math::XformTRS<float> xform{};
    CameraData camera = Perspective{};
    Extent2D size{};

    [[nodiscard]] glm::mat4 projection_matrix() const;
    [[nodiscard]] glm::mat4 view_matrix() const;
    [[nodiscard]] Frustum frustum_ws() const;
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
    void render(const RenderOptions &options) override;

    ITexture *get_color_texture() override;

    std::vector<uint64_t> query_selection(uint32_t x,
                                          uint32_t y,
                                          uint32_t width,
                                          uint32_t height) override;
    IOverlay *overlay() override;

    IEffect *effect() override;
    Effect *effect_vk();

    void debug_gui() override;
    [[nodiscard]] OverlayRenderer *vk_overlay() const;

    [[nodiscard]] Context *context() const;
    [[nodiscard]] Scene *scene_vk() const;

    [[nodiscard]] Handle<Texture> render_target(NamedRT name) const;
    [[nodiscard]] RenderTargetManager *rt_manager() const;
    [[nodiscard]] RenderTargetId rt_id(NamedRT name) const;
    [[nodiscard]] TextureCreateInfo rt_info(NamedRT name) const;
    [[nodiscard]] std::unique_ptr<RenderPass> create_single_pass_render_pass(
        NamedRT *colors, uint32_t color_count, NamedRT depth_stencil) const;

    [[nodiscard]] Drawer *drawer() const;

    [[nodiscard]] ViewData data() const;
    [[nodiscard]] ViewData last_frame_data() const;

    [[nodiscard]] Handle<Buffer> transform_buffer();

  private:
    void flip_history_buffer();
    void alloc_render_targets();
    void update_color_tex_adapter(NamedRT rt);
    void update_transform_buffer();
    [[nodiscard]] TextureInfo color_tex_info() const;

    Scene *_scene = nullptr;
    ViewData _data{};
    std::optional<ViewData> _last_frame_data{};

    std::unique_ptr<RenderTargetManager> _rt_manager{};
    RenderTargetId _rt_ids[NamedRT_Count]{};

    std::unique_ptr<TextureAdapter> _color_tex_adapter{};

    std::unique_ptr<Renderer> _renderer{};
    std::unique_ptr<OverlayRenderer> _overlay_renderer{};
    std::unique_ptr<Drawer> _drawer{};
    std::unique_ptr<Effect> _effect{};

    Handle<Buffer> _transform_buffer{};
};
} // namespace ars::render::vk