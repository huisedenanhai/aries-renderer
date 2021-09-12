#pragma once

#include "Vulkan.h"

namespace ars::render::vk {
class Context;

class Shader {
  public:
    Shader(Context *context, const char *name);

    ARS_NO_COPY_MOVE(Shader);

    ~Shader();

    [[nodiscard]] VkShaderModule module() const;

  private:
    Context *_context = nullptr;
    VkShaderModule _module = VK_NULL_HANDLE;
};
} // namespace ars::render::vk