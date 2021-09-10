#pragma once

#include "Common.h"
#include <memory>
#include <optional>
#include <string>

struct GLFWwindow;

namespace ars::render {
class ISwapchain;
class ITexture;
class IScene;
class IMesh;
class IMaterial;

struct TextureInfo;

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

    std::unique_ptr<ITexture> create_texture(const TextureInfo &info);

    std::unique_ptr<ITexture>
    create_texture_2d(Format format,
                      uint32_t width,
                      uint32_t height,
                      uint32_t mip_levels = MAX_MIP_LEVELS);

    virtual std::unique_ptr<IScene> create_scene() = 0;
    virtual std::unique_ptr<IMesh> create_mesh() = 0;
    virtual std::unique_ptr<IMaterial> create_material() = 0;

    // Call this method when a frame begins. If this method returns false,
    // the backend refuse to begin a new frame and no render work should be
    // submitted. If this method returns true, a call to end_frame() is required
    // to denote the end of the frame.
    //
    // begin_frame() and end_frame() are hints to backend for synchronization.
    //
    // e.g.
    // if (ctx->begin_frame()) {
    //   // submit your render works here
    //   ctx->end_frame();
    // }
    virtual bool begin_frame() = 0;
    virtual void end_frame() = 0;

  protected:
    virtual std::unique_ptr<ITexture>
    create_texture_impl(const TextureInfo &info) = 0;
};

} // namespace ars::render