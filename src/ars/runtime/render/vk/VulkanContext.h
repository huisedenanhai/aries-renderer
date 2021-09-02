#pragma once

#include "../IRenderContext.h"

namespace ars::render {
class VulkanContext : public IRenderContext {
  public:
    VulkanContext();
    std::unique_ptr<ISwapchain>
    create_swapchain(uint64_t window_handle) override;
    std::unique_ptr<IBuffer> create_buffer() override;
    std::unique_ptr<ITexture> create_texture() override;
    std::unique_ptr<IScene> create_scene() override;
    std::unique_ptr<IMesh> create_mesh() override;
    std::unique_ptr<IMaterial> create_material() override;
};
} // namespace ars::render
