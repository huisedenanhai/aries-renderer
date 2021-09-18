#include "Swapchain.h"
#include "Context.h"
#include "ImGui.h"
#include "Pipeline.h"
#include <GLFW/glfw3.h>
#include <algorithm>
#include <ars/runtime/core/Log.h>
#include <cassert>

namespace ars::render::vk {
Swapchain::Swapchain(Context *context, VkSurfaceKHR surface, GLFWwindow *window)
    : _context(context), _surface(surface), _window(window) {
    assert(context != nullptr);
    assert(surface != VK_NULL_HANDLE);
    assert(window != nullptr);

    recreate_swapchain();

    _imgui = std::make_unique<ImGuiPass>(this);
}

Swapchain::~Swapchain() {
    _context->queue()->flush();

    _imgui.reset();

    auto device = _context->device();

    _pipeline.reset();

    if (_render_pass != VK_NULL_HANDLE) {
        device->Destroy(_render_pass);
    }

    cleanup_swapchain();

    _context->instance()->Destroy(_surface);

    if (_window != nullptr) {
        glfwDestroyWindow(_window);
    }
}

namespace {
VkSurfaceFormatKHR choose_surface_format(
    const std::vector<VkSurfaceFormatKHR> &available_formats) {
    for (const auto &available_format : available_formats) {
        if (available_format.format == VK_FORMAT_B8G8R8A8_SRGB &&
            available_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return available_format;
        }
    }
    return available_formats[0];
}

VkPresentModeKHR choose_present_mode(
    const std::vector<VkPresentModeKHR> &available_present_modes) {
    for (const auto &mode : available_present_modes) {
        if (mode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return mode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D choose_swapchain_extent(const VkSurfaceCapabilitiesKHR &capabilities,
                                   VkExtent2D target_extent) {
    if (capabilities.currentExtent.width != UINT32_MAX) {
        return capabilities.currentExtent;
    }

    VkExtent2D actual_extent = target_extent;

    actual_extent.width = std::clamp(actual_extent.width,
                                     capabilities.minImageExtent.width,
                                     capabilities.maxImageExtent.width);
    actual_extent.height = std::clamp(actual_extent.height,
                                      capabilities.minImageExtent.height,
                                      capabilities.maxImageExtent.height);

    return actual_extent;
}
} // namespace

SwapChainSupportDetails
query_swapchain_support(Instance *instance,
                        VkPhysicalDevice physical_device,
                        VkSurfaceKHR surface) {
    SwapChainSupportDetails details;
    instance->GetPhysicalDeviceSurfaceCapabilitiesKHR(
        physical_device, surface, &details.capabilities);

    uint32_t format_count = 0;
    instance->GetPhysicalDeviceSurfaceFormatsKHR(
        physical_device, surface, &format_count, nullptr);

    if (format_count != 0) {
        details.formats.resize(format_count);
        instance->GetPhysicalDeviceSurfaceFormatsKHR(
            physical_device, surface, &format_count, details.formats.data());
    }

    uint32_t present_mode_count = 0;
    instance->GetPhysicalDeviceSurfacePresentModesKHR(
        physical_device, surface, &present_mode_count, nullptr);

    if (present_mode_count != 0) {
        details.present_modes.resize(present_mode_count);
        instance->GetPhysicalDeviceSurfacePresentModesKHR(
            physical_device,
            surface,
            &present_mode_count,
            details.present_modes.data());
    }

    return details;
}

void Swapchain::init_swapchain() {
    Device *device = _context->device();
    VkPhysicalDevice physical_device = device->physical_device();

    SwapChainSupportDetails swapchain_support = query_swapchain_support(
        _context->instance(), physical_device, _surface);
    VkPresentModeKHR present_mode =
        choose_present_mode(swapchain_support.present_modes);

    if (_format.format == VK_FORMAT_UNDEFINED) {
        _format = choose_surface_format(swapchain_support.formats);
    }

    uint32_t image_count = swapchain_support.capabilities.minImageCount + 1;
    if (swapchain_support.capabilities.maxImageCount > 0 &&
        image_count > swapchain_support.capabilities.maxImageCount) {
        image_count = swapchain_support.capabilities.maxImageCount;
    }

    auto target_extent = get_target_extent();

    VkSwapchainCreateInfoKHR create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    create_info.surface = _surface;
    create_info.minImageCount = image_count;
    create_info.imageFormat = _format.format;
    create_info.imageColorSpace = _format.colorSpace;
    create_info.imageExtent = target_extent;
    create_info.imageArrayLayers = 1;
    create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    // presentation only happens on the primary queue
    create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    create_info.queueFamilyIndexCount = 0;     // Optional
    create_info.pQueueFamilyIndices = nullptr; // Optional

    create_info.preTransform = swapchain_support.capabilities.currentTransform;
    create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    create_info.presentMode = present_mode;
    create_info.clipped = VK_TRUE;
    create_info.oldSwapchain = VK_NULL_HANDLE;

    if (device->Create(&create_info, &_swapchain) != VK_SUCCESS) {
        panic("Failed to create swap chain!");
    }

    uint32_t swapchain_image_count = 0;
    device->GetSwapchainImagesKHR(_swapchain, &swapchain_image_count, nullptr);
    _images.resize(swapchain_image_count);
    device->GetSwapchainImagesKHR(
        _swapchain, &swapchain_image_count, _images.data());

    _extent = target_extent;
}

void Swapchain::init_image_views() {
    _image_views.resize(_images.size());
    _subresource_range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    _subresource_range.baseMipLevel = 0;
    _subresource_range.levelCount = 1;
    _subresource_range.baseArrayLayer = 0;
    _subresource_range.layerCount = 1;

    for (int i = 0; i < _images.size(); i++) {
        VkImageViewCreateInfo create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        create_info.image = _images[i];
        create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        create_info.format = _format.format;
        create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        create_info.subresourceRange = _subresource_range;

        if (_context->device()->Create(&create_info, &_image_views[i]) !=
            VK_SUCCESS) {
            panic("Failed to create image views!");
        }
    }
}

void Swapchain::cleanup_swapchain() {
    Device *device = _context->device();

    if (_image_ready_semaphore != VK_NULL_HANDLE) {
        device->Destroy(_image_ready_semaphore);
    }

    for (auto fb : _framebuffers) {
        device->Destroy(fb);
    }
    _framebuffers.clear();

    for (auto image_view : _image_views) {
        device->Destroy(image_view);
    }
    _image_views.clear();

    if (_swapchain != VK_NULL_HANDLE) {
        device->Destroy(_swapchain);
    }
}

void Swapchain::present(ITexture *texture) {
    auto device = _context->device();
    auto queue = _context->queue();

    if (need_recreate()) {
        queue->flush();
        recreate_swapchain();
    }

    uint32_t image_index = 0;
    auto acquire_result = device->AcquireNextImageKHR(_swapchain,
                                                      UINT64_MAX,
                                                      _image_ready_semaphore,
                                                      VK_NULL_HANDLE,
                                                      &image_index);
    if (acquire_result == VK_ERROR_OUT_OF_DATE_KHR) {
        // Skip this frame
        return;
    }

    if (acquire_result != VK_SUCCESS && acquire_result != VK_SUBOPTIMAL_KHR) {
        panic("Failed to acquire swap chain image!");
    }

    queue->submit_once(
        [&](CommandBuffer *cmd) {
            VkRenderPassBeginInfo rp_info{
                VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
            rp_info.renderPass = _render_pass;
            rp_info.framebuffer = _framebuffers[image_index];
            rp_info.renderArea = VkRect2D{{0, 0}, _extent};

            VkClearValue clear_value{};
            clear_value.color.float32[0] = 0.0;
            clear_value.color.float32[1] = 0.0;
            clear_value.color.float32[2] = 0.0;
            clear_value.color.float32[3] = 1.0;

            rp_info.clearValueCount = 1;
            rp_info.pClearValues = &clear_value;

            if (texture != nullptr) {
                cmd->BeginRenderPass(&rp_info, VK_SUBPASS_CONTENTS_INLINE);
                VkViewport viewport{0,
                                    0,
                                    static_cast<float>(_extent.width),
                                    static_cast<float>(_extent.height),
                                    0.0,
                                    1.0f};
                cmd->SetViewport(0, 1, &viewport);
                VkRect2D scissor{{0, 0}, _extent};
                cmd->SetScissor(0, 1, &scissor);

                cmd->BindPipeline(VK_PIPELINE_BIND_POINT_GRAPHICS,
                                  _pipeline->pipeline());

                auto desc_set = _pipeline->alloc_desc_set(0);

                VkWriteDescriptorSet write{};
                VkDescriptorImageInfo image_info{};
                fill_combined_image_sampler(
                    &write, &image_info, desc_set, 0, upcast(texture).get());

                device->UpdateDescriptorSets(1, &write, 0, nullptr);

                cmd->BindDescriptorSets(VK_PIPELINE_BIND_POINT_GRAPHICS,
                                        _pipeline->pipeline_layout(),
                                        0,
                                        1,
                                        &desc_set,
                                        0,
                                        nullptr);

                cmd->Draw(3, 1, 0, 0);
            }

            if (_imgui_callback.has_value()) {
                _imgui->new_frame();
                _imgui_callback.value()();
                _imgui->draw(cmd);
            }

            cmd->EndRenderPass();
        },
        1,
        &_image_ready_semaphore);

    auto blit_finish_sem = queue->get_semaphore();

    VkPresentInfoKHR present_info{VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = &blit_finish_sem;
    present_info.swapchainCount = 1;
    present_info.pSwapchains = &_swapchain;
    present_info.pImageIndices = &image_index;
    VkResult present_result =
        _context->device()->QueuePresentKHR(queue->queue(), &present_info);

    if (present_result != VK_SUCCESS &&
        present_result != VK_ERROR_OUT_OF_DATE_KHR &&
        present_result != VK_SUBOPTIMAL_KHR) {
        panic("Failed to present to swapchain");
    }

#ifdef __APPLE__
    // Dirty Fix: It crashes on macOS when multiple swapchains want to present
    // in the same frame.
    // Just flush the queue. Synchronization overhead is better than nothing.
    _context->queue()->flush();
#endif
}

bool Swapchain::need_recreate() const {
    auto target_extent = get_target_extent();
    if (target_extent.width == 0 && target_extent.height == 0) {
        // no need to update swapchain when the window is minimized.
        return false;
    }

    return _extent.width != target_extent.width ||
           _extent.height != target_extent.height;
}

void Swapchain::recreate_swapchain() {
    cleanup_swapchain();
    init_swapchain();
    init_image_views();
    init_render_pass();
    init_pipeline();
    init_framebuffers();
    init_semaphores();
}

Extent2D Swapchain::physical_size() {
    return {_extent.width, _extent.height};
}

void Swapchain::init_framebuffers() {
    auto device = _context->device();
    auto fb_count = static_cast<int>(_image_views.size());
    _framebuffers.resize(fb_count);

    for (int i = 0; i < fb_count; i++) {
        VkFramebufferCreateInfo info{VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
        info.attachmentCount = 1;
        info.pAttachments = &_image_views[i];
        info.width = _extent.width;
        info.height = _extent.height;
        info.layers = 1;
        info.renderPass = _render_pass;

        if (device->Create(&info, &_framebuffers[i]) != VK_SUCCESS) {
            panic("Failed to create framebuffers for swapchain images");
        }
    }
}

void Swapchain::init_render_pass() {
    if (_render_pass != VK_NULL_HANDLE) {
        // No need to recreate render pass as the image format will not change
        // after initialization.
        return;
    }

    VkAttachmentDescription attachment{};
    attachment.format = _format.format;
    attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

    VkAttachmentReference color{};
    color.attachment = 0;
    color.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color;

    VkRenderPassCreateInfo info{VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
    info.attachmentCount = 1;
    info.pAttachments = &attachment;
    info.subpassCount = 1;
    info.pSubpasses = &subpass;

    if (_context->device()->Create(&info, &_render_pass) != VK_SUCCESS) {
        panic("Failed to create render pass for presentation");
    }
}

void Swapchain::init_semaphores() {
    VkSemaphoreCreateInfo info{VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
    if (_context->device()->Create(&info, &_image_ready_semaphore) !=
        VK_SUCCESS) {
        panic("Failed to create image ready semaphore");
    }
}

void Swapchain::init_pipeline() {
    if (_pipeline != VK_NULL_HANDLE) {
        // No need to recreate pipeline as _render_pass will not change after
        // initialization
        return;
    }

    auto vert_shader = std::make_unique<Shader>(_context, "Blit.vert");
    auto frag_shader = std::make_unique<Shader>(_context, "Blit.frag");

    GraphicsPipelineInfo info{};
    info.shaders.push_back(vert_shader.get());
    info.shaders.push_back(frag_shader.get());
    info.render_pass = _render_pass;
    info.subpass = 0;

    _pipeline = std::make_unique<GraphicsPipeline>(_context, info);
}

VkExtent2D Swapchain::get_target_extent() const {
    int physical_width, physical_height;
    glfwGetFramebufferSize(_window, &physical_width, &physical_height);

    SwapChainSupportDetails swapchain_support = query_swapchain_support(
        _context->instance(), _context->device()->physical_device(), _surface);

    return choose_swapchain_extent(swapchain_support.capabilities,
                                   {static_cast<uint32_t>(physical_width),
                                    static_cast<uint32_t>(physical_height)});
}

bool Swapchain::should_close() {
    return glfwWindowShouldClose(_window);
}

VkRenderPass Swapchain::render_pass() const {
    return _render_pass;
}

Context *Swapchain::context() const {
    return _context;
}

GLFWwindow *Swapchain::window() const {
    return _window;
}

void Swapchain::set_imgui_callback(
    std::optional<std::function<void()>> callback) {
    _imgui_callback = std::move(callback);
}

} // namespace ars::render::vk
