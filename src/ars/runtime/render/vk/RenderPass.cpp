#include "RenderPass.h"
#include "Context.h"
#include <ars/runtime/core/Log.h>
#include <cassert>

namespace ars::render::vk {
RenderPass::RenderPass(Context *context, const VkRenderPassCreateInfo &info)
    : _context(context) {
    if (_context->device()->Create(&info, &_render_pass) != VK_SUCCESS) {
        ARS_LOG_CRITICAL("Failed to create render pass");
    }
    _attachments.reserve(info.attachmentCount);
    for (int i = 0; i < info.attachmentCount; i++) {
        _attachments.push_back(info.pAttachments[i]);
    }

    _subpasses.reserve(info.subpassCount);
    for (int i = 0; i < info.subpassCount; i++) {
        _subpasses.push_back(info.pSubpasses[i]);
    }
}

RenderPass::~RenderPass() {
    if (_render_pass != VK_NULL_HANDLE) {
        _context->device()->Destroy(_render_pass);
    }
}

VkRenderPass RenderPass::render_pass() const {
    return _render_pass;
}

RenderPassExecution RenderPass::begin(CommandBuffer *cmd,
                                      Framebuffer *framebuffer,
                                      const VkClearValue *clear_values,
                                      VkSubpassContents contents) {
    VkRenderPassBeginInfo rp_begin{VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
    rp_begin.renderPass = _render_pass;
    rp_begin.framebuffer = framebuffer->framebuffer();
    rp_begin.clearValueCount =
        clear_values == nullptr
            ? 0
            : static_cast<uint32_t>(framebuffer->attachments().size());
    rp_begin.pClearValues = clear_values;
    rp_begin.renderArea = {{0, 0}, framebuffer->extent()};

    cmd->BeginRenderPass(&rp_begin, contents);

    framebuffer->set_viewport_scissor(cmd);

    RenderPassExecution execution{};
    execution.command_buffer = cmd;
    execution.framebuffer = framebuffer;
    return execution;
}

void RenderPass::end(const RenderPassExecution &execution) {
    auto cmd = execution.command_buffer;
    auto fb = execution.framebuffer;
    assert(cmd != nullptr);
    assert(fb != nullptr);

    cmd->EndRenderPass();

    const auto &fb_attaches = fb->attachments();
    for (int i = 0; i < _attachments.size(); i++) {
        auto final_layout = _attachments[i].finalLayout;
        fb_attaches[i]->assure_layout(final_layout);
    }
}

Context *RenderPass::context() const {
    return _context;
}

const std::vector<VkAttachmentDescription> &RenderPass::attachments() const {
    return _attachments;
}

const std::vector<VkSubpassDescription> &RenderPass::subpasses() const {
    return _subpasses;
}

std::unique_ptr<RenderPass> RenderPass::create_with_single_pass(
    Context *context,
    uint32_t color_count,
    const RenderPassAttachmentInfo *color_info,
    const RenderPassAttachmentInfo *depth_stencil_info) {
    auto attach_count = color_count;
    bool has_depth_stencil = depth_stencil_info != nullptr;
    if (has_depth_stencil) {
        attach_count += 1;
    }

    std::vector<VkAttachmentDescription> attachments{};
    attachments.resize(attach_count);

    auto init_attach_desc = [&](VkAttachmentDescription &desc,
                                const RenderPassAttachmentInfo &info) {
        desc.format = info.format;
        desc.samples = info.samples;
        desc.loadOp = info.load_op;
        desc.storeOp = info.store_op;
        desc.initialLayout = info.initial_layout;
        desc.finalLayout = info.final_layout;
    };

    for (int i = 0; i < color_count; i++) {
        init_attach_desc(attachments[i], color_info[i]);
    }
    if (has_depth_stencil) {
        init_attach_desc(attachments.back(), *depth_stencil_info);
    }

    std::vector<VkAttachmentReference> color_refs{};
    color_refs.resize(color_count);
    for (int i = 0; i < color_count; i++) {
        auto &ref = color_refs[i];
        ref.attachment = i;
        ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    }

    VkAttachmentReference depth_ref{};
    depth_ref.attachment = color_count;
    depth_ref.layout = VK_IMAGE_LAYOUT_GENERAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = color_count;
    subpass.pColorAttachments = color_refs.data();
    if (has_depth_stencil) {
        subpass.pDepthStencilAttachment = &depth_ref;
    }

    VkRenderPassCreateInfo info{VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
    info.attachmentCount = static_cast<uint32_t>(std::size(attachments));
    info.pAttachments = attachments.data();
    info.subpassCount = 1;
    info.pSubpasses = &subpass;

    return std::make_unique<RenderPass>(context, info);
}

RenderPassExecution
RenderPass::begin(CommandBuffer *cmd,
                  const std::vector<Handle<Texture>> &attachments,
                  VkSubpassContents contents) {
    auto fb = _context->create_tmp_framebuffer(this, attachments);
    std::vector<VkClearValue> clear_values{};
    clear_values.resize(attachments.size());
    return begin(cmd, fb, clear_values.data(), contents);
}

Framebuffer::~Framebuffer() {
    auto device = _context->device();
    if (_framebuffer != VK_NULL_HANDLE) {
        device->Destroy(_framebuffer);
    }
}

Framebuffer::Framebuffer(RenderPass *render_pass,
                         std::vector<Handle<Texture>> attachments)
    : _context(render_pass->context()), _attachments(std::move(attachments)) {
    init_framebuffer(render_pass->render_pass());
}

void Framebuffer::init_framebuffer(VkRenderPass render_pass) {
    if (_attachments.empty()) {
        ARS_LOG_CRITICAL(
            "There must be at least one attachment for the framebuffer");
    }
    auto extent = _attachments[0]->info().extent;
    VkFramebufferCreateInfo fb_info{VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
    fb_info.width = extent.width;
    fb_info.height = extent.height;
    fb_info.renderPass = render_pass;
    fb_info.layers = 1;

    std::vector<VkImageView> attachments{};
    attachments.reserve(_attachments.size());
    for (auto &attach : _attachments) {
        attachments.push_back(attach->image_view_of_level(0));
    }
    fb_info.attachmentCount = static_cast<uint32_t>(std::size(attachments));
    fb_info.pAttachments = attachments.data();

    auto device = _context->device();
    if (device->CreateFramebuffer(&fb_info, &_framebuffer) != VK_SUCCESS) {
        ARS_LOG_CRITICAL("Failed to create framebuffer");
    }

    _extent.width = fb_info.width;
    _extent.height = fb_info.height;
}

const std::vector<Handle<Texture>> &Framebuffer::attachments() const {
    return _attachments;
}

VkExtent2D Framebuffer::extent() const {
    return _extent;
}

VkFramebuffer Framebuffer::framebuffer() const {
    return _framebuffer;
}

void Framebuffer::set_viewport_scissor(CommandBuffer *cmd) const {
    VkViewport viewport;
    viewport.x = 0;
    viewport.y = 0;
    viewport.width = (float)_extent.width;
    viewport.height = (float)_extent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    cmd->SetViewport(0, 1, &viewport);

    VkRect2D scissor{{0, 0}, _extent};
    cmd->SetScissor(0, 1, &scissor);
}
} // namespace ars::render::vk