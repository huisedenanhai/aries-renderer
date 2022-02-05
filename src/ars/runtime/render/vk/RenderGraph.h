#pragma once

#include "Texture.h"
#include "View.h"
#include <vector>

namespace ars::render::vk {
class Texture;

struct PassDependency {
    Handle<Texture> texture{};
    VkAccessFlags access_mask{};
    VkImageLayout layout{};
    VkPipelineStageFlags stage_mask{};

    // Textures in the src vector should be unique, so does the dst vector
    // Textures should be not null
    static void barrier(CommandBuffer *cmd,
                        const std::vector<PassDependency> &src_deps,
                        const std::vector<PassDependency> &dst_deps);
};

class IRenderGraphPass {
  public:
    virtual ~IRenderGraphPass() = default;

    [[nodiscard]] virtual std::vector<PassDependency> src_dependencies() {
        return {};
    }

    [[nodiscard]] virtual std::vector<PassDependency> dst_dependencies() {
        return {};
    }

    virtual void execute(CommandBuffer *cmd) = 0;
};

struct RenderGraphPassBuilder;

class RenderGraphPass : public IRenderGraphPass {
  public:
    explicit RenderGraphPass(
        std::function<void(CommandBuffer *)> execute_action);
    std::vector<PassDependency> src_dependencies() override;
    std::vector<PassDependency> dst_dependencies() override;
    void execute(CommandBuffer *cmd) override;

  private:
    friend RenderGraphPassBuilder;

    std::vector<PassDependency> _src_dependencies{};
    std::vector<PassDependency> _dst_dependencies{};
    std::function<void(CommandBuffer *)> _execute_action{};
};

struct PassDependencyBuilder {
  public:
    PassDependencyBuilder(View *view, std::vector<PassDependency> *deps);

    void add(NamedRT rt,
             VkAccessFlags access_mask,
             VkPipelineStageFlags stage_mask,
             VkImageLayout layout = VK_IMAGE_LAYOUT_GENERAL);

    void add(const Handle<Texture> &rt,
             VkAccessFlags access_mask,
             VkPipelineStageFlags stage_mask,
             VkImageLayout layout = VK_IMAGE_LAYOUT_GENERAL);

    void add(RenderTargetId rt,
             VkAccessFlags access_mask,
             VkPipelineStageFlags stage_mask,
             VkImageLayout layout = VK_IMAGE_LAYOUT_GENERAL);

  private:
    View *_view = nullptr;
    std::vector<PassDependency> *_deps{};
};

struct RenderGraphPassBuilder {
  public:
    RenderGraphPassBuilder(View *view, RenderGraphPass *pass);

    PassDependencyBuilder &src_dependencies();
    PassDependencyBuilder &dst_dependencies();

    template <typename Tex>
    RenderGraphPassBuilder &compute_shader_read(Tex &&rt) {
        return read(std::forward<Tex>(rt),
                    VK_ACCESS_SHADER_READ_BIT,
                    VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
    }

    template <typename Tex>
    RenderGraphPassBuilder &compute_shader_write(Tex &&rt) {
        return write(std::forward<Tex>(rt),
                     VK_ACCESS_SHADER_WRITE_BIT,
                     VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
    }

    template <typename Tex>
    RenderGraphPassBuilder &compute_shader_read_write(Tex &&rt) {
        return write(std::forward<Tex>(rt),
                     VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT,
                     VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
    }

    template <typename... Args> RenderGraphPassBuilder &read(Args &&...args) {
        src_dependencies().add(std::forward<Args>(args)...);
        return *this;
    }

    template <typename... Args> RenderGraphPassBuilder &write(Args &&...args) {
        src_dependencies().add(std::forward<Args>(args)...);
        dst_dependencies().add(std::forward<Args>(args)...);
        return *this;
    }

  private:
    View *_view = nullptr;
    RenderGraphPass *_pass = nullptr;
    PassDependencyBuilder _src_deps_builder;
    PassDependencyBuilder _dst_deps_builder;
};

struct RenderGraph {
  public:
    explicit RenderGraph(View *view);

    void compile();
    void execute();
    void add_pass_internal(std::unique_ptr<IRenderGraphPass> pass);

    template <typename SetUp, typename Exec>
    void add_pass(SetUp &&set_up, Exec &&exec) {
        auto pass = std::make_unique<RenderGraphPass>(exec);
        RenderGraphPassBuilder builder(_view, pass.get());
        set_up(builder);
        add_pass_internal(std::move(pass));
    }

  private:
    View *_view = nullptr;
    std::vector<std::unique_ptr<IRenderGraphPass>> _passes{};
};
} // namespace ars::render::vk