#pragma once

#include "../ISwapchain.h"
#include "../ITexture.h"
#include "Vulkan.h"

namespace ars::render::vk {
class Context;

class Swapchain : public ISwapchain {
  public:
    Swapchain(Context *context, VkSurfaceKHR surface);
    ~Swapchain() override;

    void present(ITexture *texture) override;
    void resize(int physical_width, int physical_height) override;

  private:
    Context *_context = nullptr;
    VkSurfaceKHR _surface = VK_NULL_HANDLE;
};
} // namespace ars::render::vk
