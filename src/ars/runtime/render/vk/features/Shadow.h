#pragma once

#include "../RenderGraph.h"
#include "../View.h"

namespace ars::render::vk {
struct CullingResult;

constexpr uint32_t SHADOW_CASCADE_COUNT = 4;

struct ShadowCascadeData {
    glm::mat4 view_to_shadow_hclip;
    // Apply one pixel clamping to avoid atlas interpolation artifact
    // {x_min, y_min, x_max, y_max}
    glm::vec4 uv_clamp;
    float z_near;
    float z_far;
    ARS_PADDING_FIELD(float);
    ARS_PADDING_FIELD(float);
};

struct ShadowData {
    ShadowCascadeData cascades[SHADOW_CASCADE_COUNT];
};

struct SampleDistribution {
    float z_near = 0.0f;
    float z_far = 0.0f;
};

class ShadowMap {
  public:
    ShadowMap(Context *context, uint32_t resolution);
    void render(const math::XformTRS<float> &xform,
                RenderGraph &rg,
                const CullingResult &culling_result,
                const SampleDistribution &sample_dist);

    Handle<Texture> texture() const;

    ShadowData data(View *view) const;

  private:
    struct CascadeCamera {
        math::XformTRS<float> xform{};
        Orthographic camera{};
        float partition_z_near{};
        float partition_z_far{};

        struct Viewport {
            float x, y, w, h;
        } viewport{};
    };

    void update_cascade_cameras(const math::XformTRS<float> &xform,
                                View *view,
                                const SampleDistribution &sample_dist);

    Context *_context = nullptr;
    Handle<Texture> _texture{};
    float _slope_bias = 1.0f;
    float _constant_bias = 100.0f;
    CascadeCamera _cascade_cameras[SHADOW_CASCADE_COUNT]{};
};

class Shadow {
  public:
    explicit Shadow(View *view);
    void render(RenderGraph &rg, const CullingResult &culling_result);
    void read_back_hiz(RenderGraph &rg);

  private:
    SampleDistribution calculate_sample_distribution();

    View *_view = nullptr;
    Handle<Buffer> _last_frame_hiz_data{};
    // Width and height in pixels
    uint32_t _hiz_buffer_width = 0;
    uint32_t _hiz_buffer_height = 0;
    uint32_t _hiz_buffer_capture_level = 6; // Split screen into 64x64 block
};
} // namespace ars::render::vk