#include "Pipeline.h"
#include "Context.h"
#include <ars/runtime/core/Log.h>
#include <sstream>

namespace ars::render::vk {
Shader::Shader(Context *context, const char *name) : _context(context) {
    auto code = load_spirv_code(name, nullptr, 0);

    VkShaderModuleCreateInfo info{VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
    info.pCode = reinterpret_cast<const uint32_t *>(code.data);
    info.codeSize = code.size;

    if (_context->device()->Create(&info, &_module) != VK_SUCCESS) {
        std::stringstream ss;
        ss << "Failed to load shader module " << name;
        panic(ss.str());
    }
}

Shader::~Shader() {
    if (_module != VK_NULL_HANDLE) {
        _context->device()->Destroy(_module);
    }
}

VkShaderModule Shader::module() const {
    return _module;
}
} // namespace ars::render::vk
