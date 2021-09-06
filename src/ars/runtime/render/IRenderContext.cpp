#include "IRenderContext.h"
#include "vk/Context.h"
#include <ars/runtime/core/Log.h>

namespace ars::render {
namespace {
std::unique_ptr<ApplicationInfo> s_application_info{};
}

void init_render_backend(const ApplicationInfo &info) {
    if (s_application_info != nullptr) {
        log_error("Can not init render backend twice");
        return;
    }

    s_application_info = std::make_unique<ApplicationInfo>(info);

    switch (info.backend) {
    case Backend::Vulkan:
        vk::init_vulkan_backend(info);
        break;
    }
}

void destroy_render_backend() {
    if (s_application_info == nullptr) {
        log_warn("Backend is not init, but you want to destroy it.");
        return;
    }
    switch (s_application_info->backend) {
    case Backend::Vulkan:
        vk::destroy_vulkan_backend();
        break;
    }
    s_application_info.reset();
}

std::unique_ptr<IRenderContext> IRenderContext::create(GLFWwindow *window) {
    if (s_application_info == nullptr) {
        log_error("Render backend has not been initialized. Please call "
                  "init_render_backend first");
        return nullptr;
    }

    std::unique_ptr<IRenderContext> result{};
    switch (s_application_info->backend) {
    case Backend::Vulkan:
        result = std::make_unique<vk::Context>(window);
        break;
    }
    return result;
}
} // namespace ars::render