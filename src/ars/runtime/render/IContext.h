#pragma once

#include <memory>
#include <optional>
#include <string>

struct GLFWwindow;

namespace ars::render {
class ISwapchain;
class IBuffer;
class ITexture;
class IScene;
class IMesh;
class IMaterial;

enum class Backend { Vulkan };

struct ApplicationInfo {
    Backend backend = Backend::Vulkan;
    std::string app_name{};

    struct Version {
        int major = 0;
        int minor = 0;
        int patch = 0;
    };

    Version version{};

    bool enable_presentation = true;
    bool enable_validation = false;
};

void init_render_backend(const ApplicationInfo &info);
void destroy_render_backend();

// The root of the renderer. The factory of render resources.
// All resources should be destroyed before context is released.
class IContext {
  public:
    virtual ~IContext() = default;

    // A window is required for device selection. If no window is provided, we
    // assume the context is not used for presentation, and later calls to
    // create_swapchain always returns nullptr.
    //
    // The window handle is used as a hint.
    //
    // If the backend needs to actually create a swapchain to proceed, the
    // swapchain is created and cached, which will be returned by the next call
    // to create_swapchain with the same window handle.
    //
    // TODO the render context should not depends on GLFW, use a platform
    // specific native window handle instead.
    static std::unique_ptr<IContext> create(GLFWwindow *window);

    // This method returns nullptr if swapchain creation fails
    virtual std::unique_ptr<ISwapchain>
    create_swapchain(GLFWwindow *window) = 0;

    virtual std::unique_ptr<IBuffer> create_buffer() = 0;
    virtual std::unique_ptr<ITexture> create_texture() = 0;

    virtual std::unique_ptr<IScene> create_scene() = 0;
    virtual std::unique_ptr<IMesh> create_mesh() = 0;
    virtual std::unique_ptr<IMaterial> create_material() = 0;
};
} // namespace ars::render