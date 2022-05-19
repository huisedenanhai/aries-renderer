#include "Vulkan.h"
#include <ars/runtime/core/Log.h>
#include <frill_shaders.hpp>

namespace ars::render::vk {
MemoryView
load_spirv_code(const char *path, const char **flags, uint32_t flag_count) {
    auto code = frill_shaders::load(path, flags, flag_count);
    if (code.code == nullptr) {
        ARS_LOG_CRITICAL("Shader {} not found, have you add it to frill.json?",
                         path);
    }
    return MemoryView{code.code, code.size};
}
} // namespace ars::render::vk