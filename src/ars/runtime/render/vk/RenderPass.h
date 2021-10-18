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

class RenderPass {
  public:
    RenderPass(Context *context, const VkRenderPassCreateInfo &info);
    ~RenderPass();

    VkRenderPass render_pass() const;
    Context *context() const;

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
};

class Framebuffer {
  public:
    Framebuffer(RenderPass *render_pass,
                std::vector<Handle<Texture>> attachments);
    ~Framebuffer();

    const std::vector<Handle<Texture>> &attachments() const;
    VkExtent2D extent() const;
    VkFramebuffer framebuffer() const;

    void set_viewport_scissor(CommandBuffer *cmd) const;

  private:
    void init_framebuffer();

    RenderPass *_render_pass = nullptr;
    VkFramebuffer _framebuffer = VK_NULL_HANDLE;
    VkExtent2D _extent{};
    std::vector<Handle<Texture>> _attachments{};
};
} // namespace ars::render::vk