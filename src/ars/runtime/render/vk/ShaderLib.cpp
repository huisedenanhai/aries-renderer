#include "Vulkan.h"
#include <ars/runtime/core/Log.h>
#include <frill.h>
#include <set>
#include <sstream>

namespace ars::render::vk {
MemoryView
load_spirv_code(const char *path, const char **flags, uint32_t flag_count) {
    void *code = {};
    size_t size = {};
    std::set<std::string> unique_flags{};
    for (int i = 0; i < flag_count; i++) {
        unique_flags.emplace(flags[i]);
    }
    std::stringstream ss{};
    bool is_first = true;
    for (auto &flag : unique_flags) {
        if (is_first) {
            is_first = false;
        } else {
            ss << ",";
        }
        ss << flag;
    }
    auto uri = fmt::format("{}/flags={}", path, ss.str());
    frill::get_asset_bytes(uri.c_str(), &code, &size);
    if (code == nullptr) {
        ARS_LOG_CRITICAL("Shader {} not found, have you add it to frill.json?",
                         path);
    }
    return MemoryView{reinterpret_cast<uint8_t *>(code), size};
}
} // namespace ars::render::vk