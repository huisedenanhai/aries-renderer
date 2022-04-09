#pragma once

#include "../RenderGraph.h"
#include "../View.h"

namespace ars::render::vk {
struct CullingResult;

struct ShadowData {
    glm::mat4 view_to_shadow_hclip;
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
    void update_camera(const math::XformTRS<float> &xform,
                       View *view,
                       const CullingResult &culling_result,
                       const SampleDistribution &sample_dist);

    Context *_context = nullptr;
    Handle<Texture> _texture{};
    math::XformTRS<float> _xform{};
    Orthographic _camera{};
    float _slope_bias = 1.0f;
    float _constant_bias = 100.0f;
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