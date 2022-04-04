#pragma once

#include "../RenderGraph.h"
#include "../View.h"

namespace ars::render::vk {
struct CullingResult;

class ShadowMap {
  public:
    ShadowMap(Context *context, uint32_t resolution);
    void render(const math::XformTRS<float> &xform,
                RenderGraph &rg,
                const CullingResult &culling_result);

    Handle<Texture> texture() const;

  private:
    void update_camera(const math::XformTRS<float> &xform,
                       Scene *scene,
                       const CullingResult &culling_result);

    Context *_context = nullptr;
    Handle<Texture> _texture{};
    math::XformTRS<float> _xform{};
    Orthographic _camera{};
};

class Shadow {
  public:
    explicit Shadow(View *view);
    void render(RenderGraph &rg, const CullingResult &culling_result);

  private:
    View *_view = nullptr;
};
} // namespace ars::render::vk