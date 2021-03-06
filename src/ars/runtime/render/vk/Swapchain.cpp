#include "Swapchain.h"
#include "Context.h"
#include "ImGui.h"
#include "Pipeline.h"
#include "Profiler.h"
#include <GLFW/glfw3.h>
#include <algorithm>
#include <ars/runtime/core/Log.h>
#include <ars/runtime/core/input/Keyboard.h>
#include <ars/runtime/core/input/Mouse.h>
#include <cassert>

namespace ars::render::vk {
namespace {
int translate(input::Key key) {
    using namespace input;
    switch (key) {
    case Key::Space:
        return GLFW_KEY_SPACE;
    case Key::Apostrophe:
        return GLFW_KEY_APOSTROPHE;
    case Key::Comma:
        return GLFW_KEY_COMMA;
    case Key::Minus:
        return GLFW_KEY_MINUS;
    case Key::Period:
        return GLFW_KEY_PERIOD;
    case Key::Slash:
        return GLFW_KEY_SLASH;
    case Key::Semicolon:
        return GLFW_KEY_SEMICOLON;
    case Key::Equal:
        return GLFW_KEY_EQUAL;
    case Key::LeftBracket:
        return GLFW_KEY_LEFT_BRACKET;
    case Key::BackSlash:
        return GLFW_KEY_BACKSLASH;
    case Key::RightBracket:
        return GLFW_KEY_RIGHT_BRACKET;
    case Key::GraveAccent:
        return GLFW_KEY_GRAVE_ACCENT;
    case Key::N0:
        return GLFW_KEY_0;
    case Key::N1:
        return GLFW_KEY_1;
    case Key::N2:
        return GLFW_KEY_2;
    case Key::N3:
        return GLFW_KEY_3;
    case Key::N4:
        return GLFW_KEY_4;
    case Key::N5:
        return GLFW_KEY_5;
    case Key::N6:
        return GLFW_KEY_6;
    case Key::N7:
        return GLFW_KEY_7;
    case Key::N8:
        return GLFW_KEY_8;
    case Key::N9:
        return GLFW_KEY_9;
    case Key::A:
        return GLFW_KEY_A;
    case Key::B:
        return GLFW_KEY_B;
    case Key::C:
        return GLFW_KEY_C;
    case Key::D:
        return GLFW_KEY_D;
    case Key::E:
        return GLFW_KEY_E;
    case Key::F:
        return GLFW_KEY_F;
    case Key::G:
        return GLFW_KEY_G;
    case Key::H:
        return GLFW_KEY_H;
    case Key::I:
        return GLFW_KEY_I;
    case Key::J:
        return GLFW_KEY_J;
    case Key::K:
        return GLFW_KEY_K;
    case Key::L:
        return GLFW_KEY_L;
    case Key::M:
        return GLFW_KEY_M;
    case Key::N:
        return GLFW_KEY_N;
    case Key::O:
        return GLFW_KEY_O;
    case Key::P:
        return GLFW_KEY_P;
    case Key::Q:
        return GLFW_KEY_Q;
    case Key::R:
        return GLFW_KEY_R;
    case Key::S:
        return GLFW_KEY_S;
    case Key::T:
        return GLFW_KEY_T;
    case Key::U:
        return GLFW_KEY_U;
    case Key::V:
        return GLFW_KEY_V;
    case Key::W:
        return GLFW_KEY_W;
    case Key::X:
        return GLFW_KEY_X;
    case Key::Y:
        return GLFW_KEY_Y;
    case Key::Z:
        return GLFW_KEY_Z;
    case Key::Escape:
        return GLFW_KEY_ESCAPE;
    case Key::Enter:
        return GLFW_KEY_ENTER;
    case Key::Tab:
        return GLFW_KEY_TAB;
    case Key::Backspace:
        return GLFW_KEY_BACKSPACE;
    case Key::Insert:
        return GLFW_KEY_INSERT;
    case Key::Delete:
        return GLFW_KEY_DELETE;
    case Key::PageUp:
        return GLFW_KEY_PAGE_UP;
    case Key::PageDown:
        return GLFW_KEY_PAGE_DOWN;
    case Key::Home:
        return GLFW_KEY_HOME;
    case Key::End:
        return GLFW_KEY_END;
    case Key::CapsLock:
        return GLFW_KEY_CAPS_LOCK;
    case Key::ScrollLock:
        return GLFW_KEY_SCROLL_LOCK;
    case Key::NumLock:
        return GLFW_KEY_NUM_LOCK;
    case Key::PrintScreen:
        return GLFW_KEY_PRINT_SCREEN;
    case Key::Pause:
        return GLFW_KEY_PAUSE;
    case Key::Right:
        return GLFW_KEY_RIGHT;
    case Key::Left:
        return GLFW_KEY_LEFT;
    case Key::Down:
        return GLFW_KEY_DOWN;
    case Key::Up:
        return GLFW_KEY_UP;
    case Key::F1:
        return GLFW_KEY_F1;
    case Key::F2:
        return GLFW_KEY_F2;
    case Key::F3:
        return GLFW_KEY_F3;
    case Key::F4:
        return GLFW_KEY_F4;
    case Key::F5:
        return GLFW_KEY_F5;
    case Key::F6:
        return GLFW_KEY_F6;
    case Key::F7:
        return GLFW_KEY_F7;
    case Key::F8:
        return GLFW_KEY_F8;
    case Key::F9:
        return GLFW_KEY_F9;
    case Key::F10:
        return GLFW_KEY_F10;
    case Key::F11:
        return GLFW_KEY_F11;
    case Key::F12:
        return GLFW_KEY_F12;
    case Key::F13:
        return GLFW_KEY_F13;
    case Key::F14:
        return GLFW_KEY_F14;
    case Key::F15:
        return GLFW_KEY_F15;
    case Key::F16:
        return GLFW_KEY_F16;
    case Key::F17:
        return GLFW_KEY_F17;
    case Key::F18:
        return GLFW_KEY_F18;
    case Key::F19:
        return GLFW_KEY_F19;
    case Key::F20:
        return GLFW_KEY_F20;
    case Key::F21:
        return GLFW_KEY_F21;
    case Key::F22:
        return GLFW_KEY_F22;
    case Key::F23:
        return GLFW_KEY_F23;
    case Key::F24:
        return GLFW_KEY_F24;
    case Key::F25:
        return GLFW_KEY_F25;
    case Key::KeyPad0:
        return GLFW_KEY_KP_0;
    case Key::KeyPad1:
        return GLFW_KEY_KP_1;
    case Key::KeyPad2:
        return GLFW_KEY_KP_2;
    case Key::KeyPad3:
        return GLFW_KEY_KP_3;
    case Key::KeyPad4:
        return GLFW_KEY_KP_4;
    case Key::KeyPad5:
        return GLFW_KEY_KP_5;
    case Key::KeyPad6:
        return GLFW_KEY_KP_6;
    case Key::KeyPad7:
        return GLFW_KEY_KP_7;
    case Key::KeyPad8:
        return GLFW_KEY_KP_8;
    case Key::KeyPad9:
        return GLFW_KEY_KP_9;
    case Key::KeyPadDecimal:
        return GLFW_KEY_KP_DECIMAL;
    case Key::KeyPadDivide:
        return GLFW_KEY_KP_DIVIDE;
    case Key::KeyPadMultiply:
        return GLFW_KEY_KP_MULTIPLY;
    case Key::KeyPadSubtract:
        return GLFW_KEY_KP_SUBTRACT;
    case Key::KeyPadAdd:
        return GLFW_KEY_KP_ADD;
    case Key::KeyPadEnter:
        return GLFW_KEY_KP_ENTER;
    case Key::KeyPadEqual:
        return GLFW_KEY_KP_EQUAL;
    case Key::LeftShift:
        return GLFW_KEY_LEFT_SHIFT;
    case Key::LeftControl:
        return GLFW_KEY_LEFT_CONTROL;
    case Key::LeftAlt:
        return GLFW_KEY_LEFT_ALT;
    case Key::LeftSuper:
        return GLFW_KEY_LEFT_SUPER;
    case Key::RightShift:
        return GLFW_KEY_RIGHT_SHIFT;
    case Key::RightControl:
        return GLFW_KEY_RIGHT_CONTROL;
    case Key::RightAlt:
        return GLFW_KEY_RIGHT_ALT;
    case Key::RightSuper:
        return GLFW_KEY_RIGHT_SUPER;
    case Key::Menu:
        return GLFW_KEY_MENU;
    }
    return GLFW_KEY_UNKNOWN;
}

int translate(input::MouseButton button) {
    switch (button) {
    case input::MouseButton::Left:
        return GLFW_MOUSE_BUTTON_LEFT;
    case input::MouseButton::Middle:
        return GLFW_MOUSE_BUTTON_MIDDLE;
    case input::MouseButton::Right:
        return GLFW_MOUSE_BUTTON_RIGHT;
    }
    return -1;
}

int translate(CursorMode mode) {
    switch (mode) {
    case CursorMode::Normal:
        return GLFW_CURSOR_NORMAL;
    case CursorMode::Hidden:
        return GLFW_CURSOR_HIDDEN;
    case CursorMode::HiddenCaptured:
        return GLFW_CURSOR_DISABLED;
    }
    return GLFW_CURSOR_NORMAL;
}

template <typename Status, typename Code>
bool read_status(const Status &status, Code code) {
    auto glfw_index = translate(code);
    if (glfw_index >= 0 && glfw_index < std::size(status)) {
        return status[glfw_index];
    }
    return false;
}

template <typename Status>
void update_status(
    Status &hold, Status &press, Status &release, int code, int action) {
    if (code < 0 || code >= std::size(hold)) {
        return;
    }

    if (action == GLFW_PRESS) {
        hold[code] = true;
        press[code] = true;
    }

    if (action == GLFW_RELEASE) {
        hold[code] = false;
        release[code] = true;
    }
}

} // namespace

class Swapchain::KeyBoard : public input::IKeyBoard {
  public:
    bool is_holding(input::Key key) override {
        return read_status(_hold, key);
    }

    bool is_pressed(input::Key key) override {
        return read_status(_press, key);
    }

    bool is_released(input::Key key) override {
        return read_status(_release, key);
    }

    void process_event(int key,
                       [[maybe_unused]] int scancode,
                       int action,
                       [[maybe_unused]] int mods) {
        update_status(_hold, _press, _release, key, action);
    }

    void on_frame_ends() {
        std::fill(_press.begin(), _press.end(), false);
        std::fill(_release.begin(), _release.end(), false);
    }

  private:
    static constexpr size_t MAX_KEY_COUNT = 512;
    using Status = std::array<bool, MAX_KEY_COUNT>;

    Status _hold{};
    Status _press{};
    Status _release{};
};

class Swapchain::Mouse : public input::IMouse {
  public:
    bool is_holding(input::MouseButton button) override {
        return read_status(_hold, button);
    }

    bool is_pressed(input::MouseButton button) override {
        return read_status(_press, button);
    }

    bool is_released(input::MouseButton button) override {
        return read_status(_release, button);
    }

    glm::dvec2 cursor_position() override {
        return _curr_cursor_position;
    }

    glm::dvec2 cursor_position_delta() override {
        if (_is_first_frame) {
            return {};
        }
        return _curr_cursor_position - _prev_cursor_position;
    }

    glm::dvec2 scroll_delta() override {
        return _scroll_delta;
    }

    void on_frame_ends() {
        std::fill(_press.begin(), _press.end(), false);
        std::fill(_release.begin(), _release.end(), false);
        _scroll_delta = glm::dvec2(0.0);
        _is_first_frame = false;
        _prev_cursor_position = _curr_cursor_position;
    }

    void process_mouse_button_event(int button,
                                    int action,
                                    [[maybe_unused]] int mods) {
        update_status(_hold, _press, _release, button, action);
    }

    void process_scroll_event(double xoffset, double yoffset) {
        _scroll_delta += glm::dvec2(xoffset, yoffset);
    }

    void process_cursor_position_event(double xpos, double ypos) {
        _curr_cursor_position = glm::dvec2(xpos, ypos);
    }

  private:
    static constexpr size_t MAX_BUTTON_COUNT = 8;
    using ButtonStatus = std::array<bool, MAX_BUTTON_COUNT>;

    ButtonStatus _hold{};
    ButtonStatus _press{};
    ButtonStatus _release{};

    glm::dvec2 _scroll_delta{};
    glm::dvec2 _curr_cursor_position{};
    glm::dvec2 _prev_cursor_position{};
    bool _is_first_frame = true;
};

Swapchain::Swapchain(Context *context,
                     VkSurfaceKHR surface,
                     GLFWwindow *window,
                     bool owns_window)
    : _context(context), _surface(surface), _window(window),
      _owns_window(owns_window) {
    assert(context != nullptr);
    assert(surface != VK_NULL_HANDLE);
    assert(window != nullptr);

    recreate_swapchain();
    _keyboard = std::make_unique<KeyBoard>();
    _mouse = std::make_unique<Mouse>();

    _context->register_swapchain(this);

    glfwSetWindowUserPointer(_window, this);
    _prev_key_callback = glfwSetKeyCallback(_window, glfw_key_callback);
    _prev_mouse_button_callback =
        glfwSetMouseButtonCallback(_window, glfw_mouse_button_callback);
    _prev_scroll_callback =
        glfwSetScrollCallback(_window, glfw_scroll_callback);
    _prev_cursor_position_callback =
        glfwSetCursorPosCallback(_window, glfw_cursor_position_callback);
}

Swapchain::~Swapchain() {
    _context->queue()->flush();
    glfwSetKeyCallback(_window, _prev_key_callback);
    glfwSetMouseButtonCallback(_window, _prev_mouse_button_callback);
    glfwSetScrollCallback(_window, _prev_scroll_callback);
    glfwSetCursorPosCallback(_window, _prev_cursor_position_callback);

    glfwSetWindowUserPointer(_window, nullptr);
    _context->unregister_swapchain(this);

    _imgui.reset();
    _pipeline.reset();
    _render_pass.reset();

    cleanup_swapchain();

    _context->instance()->Destroy(_surface);

    if (_owns_window && _window != nullptr) {
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
        ARS_LOG_CRITICAL("Failed to create swap chain!");
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
            ARS_LOG_CRITICAL("Failed to create image views!");
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
    ARS_PROFILER_SAMPLE("Present", 0xFF816341);
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
        ARS_LOG_CRITICAL("Failed to acquire swap chain image!");
    }

    queue->submit_once(
        [&](CommandBuffer *cmd) {
            ARS_PROFILER_SAMPLE("Draw Swapchain", 0xFF713611);
            VkRenderPassBeginInfo rp_info{
                VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
            rp_info.renderPass = _render_pass->render_pass();
            rp_info.framebuffer = _framebuffers[image_index];
            rp_info.renderArea = VkRect2D{{0, 0}, _extent};

            VkClearValue clear_value{};
            clear_value.color.float32[0] = 0.0;
            clear_value.color.float32[1] = 0.0;
            clear_value.color.float32[2] = 0.0;
            clear_value.color.float32[3] = 1.0;

            rp_info.clearValueCount = 1;
            rp_info.pClearValues = &clear_value;

            cmd->BeginRenderPass(&rp_info, VK_SUBPASS_CONTENTS_INLINE);

            if (texture != nullptr) {
                VkViewport viewport{0,
                                    0,
                                    static_cast<float>(_extent.width),
                                    static_cast<float>(_extent.height),
                                    0.0,
                                    1.0f};
                cmd->SetViewport(0, 1, &viewport);
                VkRect2D scissor{{0, 0}, _extent};
                cmd->SetScissor(0, 1, &scissor);

                _pipeline->bind(cmd);

                DescriptorEncoder desc{};
                desc.set_texture(0, 0, upcast(texture).get());
                desc.commit(cmd, _pipeline.get());

                cmd->Draw(3, 1, 0, 0);
            }

            // ImGui
            if (_imgui != nullptr && _imgui_callback.has_value()) {
                ARS_PROFILER_SAMPLE_VK(cmd, "ImGui", 0xFF163451);
                _imgui->new_frame();
                _imgui_callback.value()();
                _imgui->end_frame();
                _imgui->draw(cmd);
            }

            if (_present_additional_draw_callback.has_value()) {
                _present_additional_draw_callback.value()(cmd);
            }

            cmd->EndRenderPass();
        },
        1,
        &_image_ready_semaphore);

    {
        ARS_PROFILER_SAMPLE("Submit", 0xFF715391);
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
            ARS_LOG_CRITICAL("Failed to present to swapchain");
        }
    }

#ifdef __APPLE__
    {
        ARS_PROFILER_SAMPLE("Queue Flush", 0xFF715391);
        // Dirty Fix: It crashes on macOS when multiple swapchains want to
        // present in the same frame. Just flush the queue. Synchronization
        // overhead is better than nothing.
        _context->queue()->flush();
    }
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
        info.renderPass = _render_pass->render_pass();

        if (device->Create(&info, &_framebuffers[i]) != VK_SUCCESS) {
            ARS_LOG_CRITICAL(
                "Failed to create framebuffers for swapchain images");
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

    _render_pass = std::make_unique<RenderPass>(_context, info);
}

void Swapchain::init_semaphores() {
    VkSemaphoreCreateInfo info{VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
    if (_context->device()->Create(&info, &_image_ready_semaphore) !=
        VK_SUCCESS) {
        ARS_LOG_CRITICAL("Failed to create image ready semaphore");
    }
}

void Swapchain::init_pipeline() {
    if (_pipeline != VK_NULL_HANDLE) {
        // No need to recreate pipeline as _render_pass will not change after
        // initialization
        return;
    }

    auto vert_shader = Shader::find_precompiled(_context, "Blit.vert");
    auto frag_shader = Shader::find_precompiled(_context, "Blit.frag");

    GraphicsPipelineInfo info{};
    info.shaders.push_back(vert_shader.get());
    info.shaders.push_back(frag_shader.get());
    info.subpass.render_pass = _render_pass.get();
    info.subpass.index = 0;

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

RenderPass *Swapchain::render_pass() const {
    return _render_pass.get();
}

Context *Swapchain::context() const {
    return _context;
}

GLFWwindow *Swapchain::window() const {
    return _window;
}

void Swapchain::set_imgui_callback(
    std::optional<std::function<void()>> callback) {
    if (_imgui == nullptr) {
        _imgui = std::make_unique<ImGuiPass>(this);
    }
    _imgui_callback = std::move(callback);
}

void Swapchain::set_present_additional_draw_callback(
    std::optional<std::function<void(CommandBuffer *)>> callback) {
    _present_additional_draw_callback = std::move(callback);
}

input::IKeyBoard *Swapchain::keyboard() {
    return _keyboard.get();
}

void Swapchain::on_frame_ends() {
    _keyboard->on_frame_ends();
    _mouse->on_frame_ends();
}

void Swapchain::glfw_key_callback(
    GLFWwindow *window, int key, int scancode, int action, int mods) {
    auto swapchain =
        reinterpret_cast<Swapchain *>(glfwGetWindowUserPointer(window));
    if (swapchain == nullptr) {
        return;
    }
    if (swapchain->_prev_key_callback != nullptr) {
        swapchain->_prev_key_callback(window, key, scancode, action, mods);
    }
    swapchain->_keyboard->process_event(key, scancode, action, mods);
}

input::IMouse *Swapchain::mouse() {
    return _mouse.get();
}

void Swapchain::glfw_mouse_button_callback(GLFWwindow *window,
                                           int button,
                                           int action,
                                           int mods) {
    auto swapchain =
        reinterpret_cast<Swapchain *>(glfwGetWindowUserPointer(window));
    if (swapchain == nullptr) {
        return;
    }
    if (swapchain->_prev_mouse_button_callback != nullptr) {
        swapchain->_prev_mouse_button_callback(window, button, action, mods);
    }
    swapchain->_mouse->process_mouse_button_event(button, action, mods);
}

void Swapchain::glfw_scroll_callback(GLFWwindow *window,
                                     double xoffset,
                                     double yoffset) {
    auto swapchain =
        reinterpret_cast<Swapchain *>(glfwGetWindowUserPointer(window));
    if (swapchain == nullptr) {
        return;
    }
    if (swapchain->_prev_scroll_callback != nullptr) {
        swapchain->_prev_scroll_callback(window, xoffset, yoffset);
    }
    swapchain->_mouse->process_scroll_event(xoffset, yoffset);
}

void Swapchain::glfw_cursor_position_callback(GLFWwindow *window,
                                              double xpos,
                                              double ypos) {
    auto swapchain =
        reinterpret_cast<Swapchain *>(glfwGetWindowUserPointer(window));
    if (swapchain == nullptr) {
        return;
    }
    if (swapchain->_prev_cursor_position_callback != nullptr) {
        swapchain->_prev_cursor_position_callback(window, xpos, ypos);
    }
    swapchain->_mouse->process_cursor_position_event(xpos, ypos);
}

CursorMode Swapchain::cursor_mode() {
    return _cursor_mode;
}

void Swapchain::set_cursor_mode(CursorMode mode) {
    if (mode == _cursor_mode) {
        return;
    }
    _cursor_mode = mode;
    glfwSetInputMode(_window, GLFW_CURSOR, translate(mode));
}

Extent2D Swapchain::logical_size() {
    int width, height;
    glfwGetWindowSize(_window, &width, &height);
    Extent2D size{};
    size.width = width;
    size.height = height;
    return size;
}
} // namespace ars::render::vk
