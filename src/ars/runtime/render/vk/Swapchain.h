#pragma once

#include "../ITexture.h"
#include "../IWindow.h"
#include "Vulkan.h"
#include <vector>

struct GLFWwindow;

namespace ars::render::vk {
class Context;
class GraphicsPipeline;

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities{};
    std::vector<VkSurfaceFormatKHR> formats{};
    std::vector<VkPresentModeKHR> present_modes{};
};

SwapChainSupportDetails query_swapchain_support(
    Instance *instance, VkPhysicalDevice physical_device, VkSurfaceKHR surface);

class Swapchain : public IWindow {
  public:
    Swapchain(Context *context, VkSurfaceKHR surface, GLFWwindow *window);
    ~Swapchain() override;

    ARS_NO_COPY_MOVE(Swapchain);

    void present(ITexture *texture) override;
    Extent2D physical_size() override;
    bool should_close() override;

  private:
    [[nodiscard]] VkExtent2D get_target_extent() const;
    [[nodiscard]] bool need_recreate() const;

    // Recreation of swapchain does not change surface format after first call
    // for initialization.
    void recreate_swapchain();
    void cleanup_swapchain();
    void init_swapchain();
    void init_image_views();
    void init_render_pass();
    void init_framebuffers();
    void init_semaphores();
    void init_pipeline();

    Context *_context = nullptr;
    GLFWwindow *_window = nullptr;
    VkSurfaceKHR _surface = VK_NULL_HANDLE;
    VkSwapchainKHR _swapchain = VK_NULL_HANDLE;

    std::vector<VkImage> _images{};
    std::vector<VkImageView> _image_views{};
    VkSurfaceFormatKHR _format{};
    VkExtent2D _extent{};
    VkImageSubresourceRange _subresource_range{};

    // Blit the image using shader
    VkRenderPass _render_pass = VK_NULL_HANDLE;
    std::unique_ptr<GraphicsPipeline> _pipeline{};
    std::vector<VkFramebuffer> _framebuffers{};

    // Now we don't support frames in flight. One semaphore is fine.
    VkSemaphore _image_ready_semaphore{};
};
} // namespace ars::render::vk
