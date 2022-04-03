#pragma once

#include "../Pipeline.h"
#include "../RenderGraph.h"
#include "../RenderPass.h"
#include "../View.h"
#include "../Vulkan.h"
#include <array>

namespace ars::render::vk {
struct CullingResult;

class OpaqueGeometry {
  public:
    explicit OpaqueGeometry(View *view);

    void render(RenderGraph &rg, const CullingResult &culling_result);
    void execute(CommandBuffer *cmd, const CullingResult &culling_result);

  private:
    SubpassInfo render_pass();
    [[nodiscard]] static std::array<NamedRT, 5> geometry_pass_rt_names();

    View *_view = nullptr;
};
} // namespace ars::render::vk