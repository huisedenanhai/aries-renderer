#include "Swapchain.h"
#include "Context.h"
#include <algorithm>
#include <ars/runtime/core/Log.h>
#include <cassert>

namespace ars::render::vk {

Swapchain::Swapchain(Context *context,
                     VkSurfaceKHR surface,
                     uint32_t physical_width,
                     uint32_t physical_height)
    : _context(context), _surface(surface) {
    assert(context != nullptr);
    assert(surface != VK_NULL_HANDLE);

    set_target_size(physical_width, physical_height);
    recreate_swapchain();
}

Swapchain::~Swapchain() {
    cleanup_swapchain();
    _context->instance()->Destroy(_surface);
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
    VkSurfaceFormatKHR surface_format =
        choose_surface_format(swapchain_support.formats);
    VkPresentModeKHR present_mode =
        choose_present_mode(swapchain_support.present_modes);

    uint32_t image_count = swapchain_support.capabilities.minImageCount + 1;
    if (swapchain_support.capabilities.maxImageCount > 0 &&
        image_count > swapchain_support.capabilities.maxImageCount) {
        image_count = swapchain_support.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    create_info.surface = _surface;
    create_info.minImageCount = image_count;
    create_info.imageFormat = surface_format.format;
    create_info.imageColorSpace = surface_format.colorSpace;
    create_info.imageExtent = _target_extent;
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

    _image_format = surface_format.format;
    _extent = _target_extent;
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
        create_info.format = _image_format;
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
    for (auto image_view : _image_views) {
        device->Destroy(image_view);
    }
    if (_swapchain != VK_NULL_HANDLE) {
        device->Destroy(_swapchain);
    }
}

void Swapchain::present(ITexture *texture) {}

void Swapchain::resize(uint32_t physical_width, uint32_t physical_height) {
    set_target_size(physical_width, physical_height);
}

void Swapchain::set_target_size(uint32_t physical_width,
                                uint32_t physical_height) {
    SwapChainSupportDetails swapchain_support = query_swapchain_support(
        _context->instance(), _context->device()->physical_device(), _surface);

    _target_extent = choose_swapchain_extent(swapchain_support.capabilities,
                                             {physical_width, physical_height});
}

bool Swapchain::need_recreate() const {
    return _extent.width != _target_extent.width ||
           _extent.height != _target_extent.height;
}

void Swapchain::recreate_swapchain() {
    cleanup_swapchain();
    init_swapchain();
    init_image_views();
}

Extent2D Swapchain::get_size() {
    // The actual size may differ as the swapchain is lazy recreated.
    // But all public operation of the swapchain will behave as if the target
    // size is immediately updated.
    return {_target_extent.width, _target_extent.height};
}

} // namespace ars::render::vk
