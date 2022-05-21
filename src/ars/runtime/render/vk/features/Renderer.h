#pragma once

#include "../Pipeline.h"
#include "../RenderGraph.h"
#include "../View.h"
#include "../Vulkan.h"
#include <memory>

namespace ars::render::vk {
class OpaqueGeometry;
class DeferredShading;
class GenerateHierarchyZ;
class ScreenSpaceReflection;
class ToneMapping;
class QuerySelection;
class Shadow;

// Value must sequentially increase
enum RenderPassID {
    RenderPassID_Geometry,
    RenderPassID_Shading,
    RenderPassID_Overlay,
    RenderPassID_Shadow,
    RenderPassID_ObjectID,
    RenderPassID_RayTracing,
    RenderPassID_Count,
};

constexpr VkFormat ID_COLOR_ATTACHMENT_FORMAT = VK_FORMAT_R32_UINT;

class RendererContextData {
  public:
    explicit RendererContextData(Context *context);
    [[nodiscard]] SubpassInfo subpass(RenderPassID pass_id) const;

  private:
    void init_render_passes();
    void init_geometry_pass();
    void init_shading_pass();
    void init_overlay_pass();
    void init_shadow_pass();
    void init_object_id_pass();

    Context *_context = nullptr;
    std::unique_ptr<RenderPass> _geometry_pass{};
    std::unique_ptr<RenderPass> _shading_pass{};
    std::unique_ptr<RenderPass> _overlay_pass{};
    std::unique_ptr<RenderPass> _shadow_pass{};
    std::unique_ptr<RenderPass> _object_id_pass{};
    std::vector<SubpassInfo> _subpasses{};
};

class Renderer {
  public:
    explicit Renderer(View *view);
    ~Renderer();

    NamedRT render(RenderGraph &rg, const RenderOptions &options);
    std::vector<uint64_t>
    query_selection(uint32_t x, uint32_t y, uint32_t width, uint32_t height);

  private:
    void add_inplace(RenderGraph &rg, NamedRT add_src_rt, NamedRT target_rt);

    View *_view = nullptr;
    std::unique_ptr<Shadow> _shadow{};
    std::unique_ptr<OpaqueGeometry> _opaque_geometry{};
    std::unique_ptr<DeferredShading> _deferred_shading{};
    std::unique_ptr<GenerateHierarchyZ> _generate_hierarchy_z{};
    std::unique_ptr<ScreenSpaceReflection> _screen_space_reflection{};
    std::unique_ptr<ToneMapping> _tone_mapping{};
    std::unique_ptr<QuerySelection> _query_selection{};

    std::unique_ptr<ComputePipeline> _add_inplace_pipeline{};
};
} // namespace ars::render::vk