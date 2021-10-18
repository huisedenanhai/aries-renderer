#pragma once

#include "../ITexture.h"
#include "../IWindow.h"
#include "Vulkan.h"
#include <vector>

struct GLFWwindow;

namespace ars::render::vk {
class Context;
class RenderPass;
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
    input::IMouse *mouse() override;
    CursorMode cursor_mode() override;
    void set_cursor_mode(CursorMode mode) override;

    // The present render pass has already begun when the callback is triggered
    void set_present_additional_draw_callback(
        std::optional<std::function<void(CommandBuffer *cmd)>> callback);
    void on_frame_ends();

    // The render pass for present
    [[nodiscard]] RenderPass *render_pass() const;
    [[nodiscard]] Context *context() const;
    [[nodiscard]] GLFWwindow *window() const;

  private:
    static void glfw_key_callback(
        GLFWwindow *window, int key, int scancode, int action, int mods);
    using GLFWKeyCallback = decltype(glfw_key_callback) *;

    static void glfw_mouse_button_callback(GLFWwindow *window,
                                           int button,
                                           int action,
                                           int mods);
    using GLFWMouseButtonCallback = decltype(glfw_mouse_button_callback) *;

    static void
    glfw_scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
    using GLFWScrollCallback = decltype(glfw_scroll_callback) *;

    static void
    glfw_cursor_position_callback(GLFWwindow *window, double xpos, double ypos);
    using GLFWCursorPositionCallback =
        decltype(glfw_cursor_position_callback) *;

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
    std::unique_ptr<RenderPass> _render_pass{};
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
    class Mouse;
    std::unique_ptr<Mouse> _mouse{};

    GLFWKeyCallback _prev_key_callback = nullptr;
    GLFWMouseButtonCallback _prev_mouse_button_callback = nullptr;
    GLFWScrollCallback _prev_scroll_callback = nullptr;
    GLFWCursorPositionCallback _prev_cursor_position_callback = nullptr;

    CursorMode _cursor_mode = CursorMode::Normal;
};
} // namespace ars::render::vk
