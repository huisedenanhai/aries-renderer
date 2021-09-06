#include "Swapchain.h"
#include "Context.h"
#include <cassert>

namespace ars::render::vk {
void Swapchain::present(ITexture *texture) {}

void Swapchain::resize(int physical_width, int physical_height) {}

Swapchain::Swapchain(Context *context, VkSurfaceKHR surface)
    : _context(context), _surface(surface) {
    assert(context != nullptr);
    assert(surface != VK_NULL_HANDLE);
}

Swapchain::~Swapchain() {
    _context->get_instance()->Destroy(_surface);
}
} // namespace ars::render
