#pragma once

#include "../ISwapchain.h"
#include "../ITexture.h"
#include "VulkanContext.h"

namespace ars::render {
class VulkanSwapchain : public ISwapchain {
  public:
    void present(ITexture *texture) override;
};
} // namespace ars::render
