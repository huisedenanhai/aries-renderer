#include "VulkanContext.h"
#include "VulkanBuffer.h"
#include "VulkanMaterial.h"
#include "VulkanMesh.h"
#include "VulkanScene.h"
#include "VulkanSwapchain.h"
#include "VulkanTexture.h"

namespace ars::render {
VulkanContext::VulkanContext() {}

std::unique_ptr<ISwapchain>
VulkanContext::create_swapchain(uint64_t window_handle) {
    return std::make_unique<VulkanSwapchain>();
}

std::unique_ptr<IBuffer> VulkanContext::create_buffer() {
    return std::make_unique<VulkanBuffer>();
}

std::unique_ptr<ITexture> VulkanContext::create_texture() {
    return std::make_unique<VulkanTexture>();
}

std::unique_ptr<IScene> VulkanContext::create_scene() {
    return std::make_unique<VulkanScene>();
}

std::unique_ptr<IMesh> VulkanContext::create_mesh() {
    return std::make_unique<VulkanMesh>();
}

std::unique_ptr<IMaterial> VulkanContext::create_material() {
    return std::make_unique<VulkanMaterial>();
}
} // namespace ars::render
