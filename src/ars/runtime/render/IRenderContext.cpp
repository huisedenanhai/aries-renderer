#include "IRenderContext.h"
#include "vk/VulkanContext.h"

namespace ars::render {
std::unique_ptr<IRenderContext>
IRenderContext::create(Backend backend, std::optional<uint64_t> window_handle) {
    std::unique_ptr<IRenderContext> result{};
    switch (backend) {
    case Backend::Vulkan:
        result = std::make_unique<VulkanContext>();
        break;
    }
    return result;
}
} // namespace ars::render