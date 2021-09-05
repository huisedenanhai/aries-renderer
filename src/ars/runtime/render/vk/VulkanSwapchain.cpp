#include "VulkanSwapchain.h"
#include "VulkanContext.h"
#include <cassert>

namespace ars::render {
void VulkanSwapchain::present(ITexture *texture) {}

void VulkanSwapchain::resize(int physical_width, int physical_height) {}

VulkanSwapchain::VulkanSwapchain(VulkanContext *context, VkSurfaceKHR surface)
    : _context(context), _surface(surface) {
    assert(context != nullptr);
    assert(surface != VK_NULL_HANDLE);
}

VulkanSwapchain::~VulkanSwapchain() {
    _context->get_instance()->Destroy(_surface);
}
} // namespace ars::render
