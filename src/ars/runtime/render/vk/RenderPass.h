#pragma once

#include "Vulkan.h"
#include <vector>

namespace ars::render::vk {
class Context;
class Framebuffer;
class Texture;

struct RenderPassExecution {
    CommandBuffer *command_buffer;
    Framebuffer *framebuffer;
};

struct RenderPassAttachmentInfo {
    VkFormat format;
    VkSampleCountFlagBits samples;
};

class RenderPass {
  public:
    RenderPass(Context *context, const VkRenderPassCreateInfo &info);
    ~RenderPass();

    static std::unique_ptr<RenderPass>
    create_with_single_pass(Context *context,
                            uint32_t color_count,
                            const RenderPassAttachmentInfo *color_info,
                            const RenderPassAttachmentInfo *depth_stencil_info);

    [[nodiscard]] VkRenderPass render_pass() const;
    [[nodiscard]] Context *context() const;
    [[nodiscard]] const std::vector<VkAttachmentDescription> &
    attachments() const;
    [[nodiscard]] const std::vector<VkSubpassDescription> &subpasses() const;

    // clear value size must match with framebuffer attachment count
    RenderPassExecution begin(CommandBuffer *cmd,
                              Framebuffer *framebuffer,
                              const VkClearValue *clear_values,
                              VkSubpassContents contents);
    void end(const RenderPassExecution &execution);

  private:
    Context *_context = nullptr;
    VkRenderPass _render_pass = VK_NULL_HANDLE;
    std::vector<VkAttachmentDescription> _attachments{};
    std::vector<VkSubpassDescription> _subpasses{};
};

class Framebuffer {
  public:
    Framebuffer(RenderPass *render_pass,
                std::vector<Handle<Texture>> attachments);
    ~Framebuffer();

    [[nodiscard]] const std::vector<Handle<Texture>> &attachments() const;
    [[nodiscard]] VkExtent2D extent() const;
    [[nodiscard]] VkFramebuffer framebuffer() const;

    void set_viewport_scissor(CommandBuffer *cmd) const;

  private:
    void init_framebuffer(VkRenderPass render_pass);

    // Cache
    Context *_context = nullptr;
    VkFramebuffer _framebuffer = VK_NULL_HANDLE;
    VkExtent2D _extent{};
    std::vector<Handle<Texture>> _attachments{};
};
} // namespace ars::render::vk