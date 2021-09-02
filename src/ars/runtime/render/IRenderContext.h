#pragma once

#include <memory>
#include <optional>

namespace ars::render {
class ISwapchain;
class IBuffer;
class ITexture;
class IScene;
class IMesh;
class IMaterial;

enum class Backend { Vulkan };

// The root of the renderer. The factory of render resources.
// All resources should be destroyed before context is released.
class IRenderContext {
  public:
    virtual ~IRenderContext() = default;

    // A window is required for device selection. If no window is provided, we
    // assume the context is not used for presentation, and later calls to
    // create_swapchain always returns nullptr.
    //
    // The window handle is used as a hint.
    //
    // If the backend needs to actually create a swapchain to proceed, the
    // swapchain is created and cached, which will be returned by the next call
    // to create_swapchain with the same window handle.
    static std::unique_ptr<IRenderContext>
    create(Backend backend, std::optional<uint64_t> window_handle);

    // This method returns nullptr if swapchain creation fails
    virtual std::unique_ptr<ISwapchain>
    create_swapchain(uint64_t window_handle) = 0;

    virtual std::unique_ptr<IBuffer> create_buffer() = 0;
    virtual std::unique_ptr<ITexture> create_texture() = 0;

    virtual std::unique_ptr<IScene> create_scene() = 0;
    virtual std::unique_ptr<IMesh> create_mesh() = 0;
    virtual std::unique_ptr<IMaterial> create_material() = 0;
};
} // namespace ars::render