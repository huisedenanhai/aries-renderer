#pragma once

#include "../Buffer.h"
#include "../Vulkan.h"

namespace ars::render::vk {
class Context;

class AccelerationStructure {
  public:
    AccelerationStructure(Context *context,
                          VkAccelerationStructureTypeKHR type,
                          VkDeviceSize buffer_size);
    ~AccelerationStructure();

    [[nodiscard]] VkAccelerationStructureKHR acceleration_structure() const;

  private:
    Context *_context = nullptr;
    VkAccelerationStructureKHR _acceleration_structure = VK_NULL_HANDLE;
    Handle<Buffer> _buffer{};
};
} // namespace ars::render::vk