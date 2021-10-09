#pragma once

#include "../ITexture.h"
#include "../IWindow.h"
#include "Vulkan.h"
#include <vector>

struct GLFWwindow;

namespace ars::render::vk {
class Context;
class GraphicsPipeline;
class ImGuiPass;

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities{};
    std::vector<VkSurfaceFormatKHR> formats{};
    std::vector<VkPresentModeKHR> present_modes{};
};

SwapChainSupportDetails query_swapchain_support(
    Instance *instance, VkPhysicalDevice physical_device, VkSurfaceKHR surface);

class Swapchain : public IWindow {
  public:
    // Some use case manages the window destruction itself (Like ImGui
    // multi-viewport), for that case set owns_window = false.
    Swapchain(Context *context,
              VkSurfaceKHR surface,
              GLFWwindow *window,
              bool owns_window);
    ~Swapchain() override;

    ARS_NO_COPY_MOVE(Swapchain);

    void present(ITexture *texture) override;
    Extent2D physical_size() override;
    bool should_close() override;
    void
    set_imgui_callback(std::optional<std::function<void()>> callback) override;
    input::IKeyBoard *keyboard() override;

    // The present render pass has already begun when the callback is triggered
    void set_present_additional_draw_callback(
        std::optional<std::function<void(CommandBuffer *cmd)>> callback);
    void on_frame_ends();

    // The render pass for present
    [[nodiscard]] VkRenderPass render_pass() const;
    [[nodiscard]] Context *context() const;
    [[nodiscard]] GLFWwindow *window() const;

  private:
    static void glfw_key_callback(
        GLFWwindow *window, int key, int scancode, int action, int mods);
    using GLFWKeyCallback = decltype(glfw_key_callback) *;

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
    bool _owns_window = false;
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

    std::unique_ptr<ImGuiPass> _imgui{};
    std::optional<std::function<void()>> _imgui_callback{};
    std::optional<std::function<void(CommandBuffer *cmd)>>
        _present_additional_draw_callback{};

    class KeyBoard;
    std::unique_ptr<KeyBoard> _keyboard{};
    GLFWKeyCallback _prev_key_callback = nullptr;
};
} // namespace ars::render::vk
