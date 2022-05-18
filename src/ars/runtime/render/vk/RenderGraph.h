#pragma once

#include "Texture.h"
#include "View.h"
#include <set>
#include <vector>

namespace ars::render::vk {
class Texture;

struct PassDependency {
    Handle<Texture> texture{};
    VkAccessFlags access_mask{};
    VkImageLayout layout{};
    VkPipelineStageFlags stage_mask{};
    // If the pass performs a write-only access to rt and all content is
    // rewrite, previous write to the rt will be considered useless and previous
    // passes write to this rt might be culled
    bool full_rewrite = true;

    // Textures in the src vector should be unique, so does the dst vector
    // Textures should be not null
    static void barrier(CommandBuffer *cmd,
                        const std::vector<PassDependency> &src_deps,
                        const std::vector<PassDependency> &dst_deps);
};

struct RenderGraphPassBuilder;

// Pass will be culled if
// 1. has no side effect
// 2. write not used by later passes or execute_action is nullptr
// A pass should be marked has side effect if it writes something other than
// given rt
class RenderGraphPass {
  public:
    [[nodiscard]] std::vector<PassDependency> src_dependencies() const;
    [[nodiscard]] std::vector<PassDependency> dst_dependencies() const;
    void execute(CommandBuffer *cmd) const;

    std::vector<PassDependency> dependencies{};
    std::function<void(CommandBuffer *)> execute_action{};
    bool has_side_effect = false;
};

struct PassDependencyBuilder {
  public:
    PassDependencyBuilder(View *view, std::vector<PassDependency> *deps);

    void add(NamedRT rt,
             VkAccessFlags access_mask,
             VkPipelineStageFlags stage_mask,
             bool full_rewrite = true,
             VkImageLayout layout = VK_IMAGE_LAYOUT_GENERAL);

    void add(const Handle<Texture> &rt,
             VkAccessFlags access_mask,
             VkPipelineStageFlags stage_mask,
             bool full_rewrite = true,
             VkImageLayout layout = VK_IMAGE_LAYOUT_GENERAL);

    void add(RenderTargetId rt,
             VkAccessFlags access_mask,
             VkPipelineStageFlags stage_mask,
             bool full_rewrite = true,
             VkImageLayout layout = VK_IMAGE_LAYOUT_GENERAL);

  private:
    View *_view = nullptr;
    std::vector<PassDependency> *_deps{};
};

struct RenderGraphPassBuilder {
  public:
    RenderGraphPassBuilder(View *view, RenderGraphPass *pass);

    template <typename Tex>
    RenderGraphPassBuilder &compute_shader_read(Tex &&rt) {
        return access(std::forward<Tex>(rt),
                      VK_ACCESS_SHADER_READ_BIT,
                      VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
    }

    template <typename Tex>
    RenderGraphPassBuilder &compute_shader_write(Tex &&rt,
                                                 bool full_rewrite = true) {
        return access(std::forward<Tex>(rt),
                      VK_ACCESS_SHADER_WRITE_BIT,
                      VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                      full_rewrite);
    }

    template <typename Tex>
    RenderGraphPassBuilder &compute_shader_read_write(Tex &&rt) {
        return access(std::forward<Tex>(rt),
                      VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT,
                      VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
    }

    template <typename... Args> RenderGraphPassBuilder &access(Args &&...args) {
        _deps_builder.add(std::forward<Args>(args)...);
        return *this;
    }

    void has_side_effect(bool side_effect);

  private:
    View *_view = nullptr;
    RenderGraphPass *_pass = nullptr;
    PassDependencyBuilder _deps_builder;
};

struct RenderGraph {
  public:
    explicit RenderGraph(View *view);

    // Mark render target as output, any pass that writes to it will not be
    // culled.
    // Use this method to mark the final rt for present and rts used in the next
    // frame.
    void output(RenderTargetId id);
    void output(NamedRT rt);
    void compile();
    void execute();

    template <typename SetUp, typename Exec>
    void add_pass(SetUp &&set_up, Exec &&exec) {
        auto pass = alloc_pass();
        RenderGraphPassBuilder builder(_view, pass);
        set_up(builder);
        pass->execute_action = exec;
    }

    View *view() const;

  private:
    RenderGraphPass *alloc_pass();

    View *_view = nullptr;

    struct PassInfo {
        RenderGraphPass pass{};
        bool active = false;
    };

    std::set<RenderTargetId> _output_rts{};
    std::vector<PassInfo> _passes{};
};
} // namespace ars::render::vk