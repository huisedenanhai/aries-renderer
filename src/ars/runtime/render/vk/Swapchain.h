#pragma once

#include "../ISwapchain.h"
#include "../ITexture.h"
#include "Vulkan.h"
#include <vector>

namespace ars::render::vk {
class Context;

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities{};
    std::vector<VkSurfaceFormatKHR> formats{};
    std::vector<VkPresentModeKHR> present_modes{};
};

SwapChainSupportDetails query_swapchain_support(
    Instance *instance, VkPhysicalDevice physical_device, VkSurfaceKHR surface);

class Swapchain : public ISwapchain {
  public:
    Swapchain(Context *context,
              VkSurfaceKHR surface,
              uint32_t physical_width,
              uint32_t physical_height);
    ~Swapchain() override;

    ARS_NO_COPY_MOVE(Swapchain);

    void present(ITexture *texture) override;
    void resize(uint32_t physical_width, uint32_t physical_height) override;
    Extent2D get_size() override;

  private:
    void set_target_size(uint32_t physical_width, uint32_t physical_height);
    bool need_recreate() const;
    void recreate_swapchain();
    void cleanup_swapchain();
    void init_swapchain();
    void init_image_views();

    Context *_context = nullptr;
    VkSurfaceKHR _surface = VK_NULL_HANDLE;
    VkSwapchainKHR _swapchain = VK_NULL_HANDLE;

    std::vector<VkImage> _images{};
    std::vector<VkImageView> _image_views{};
    VkFormat _image_format{};
    VkExtent2D _extent{};
    VkImageSubresourceRange _subresource_range{};

    // lazy swapchain recreation
    VkExtent2D _target_extent{};
};
} // namespace ars::render::vk
