#include "IContext.h"
#include "vk/Context.h"
#include "vk/Swapchain.h"
#include <algorithm>
#include <ars/runtime/core/Log.h>

namespace ars::render {
namespace {
std::unique_ptr<ApplicationInfo> s_application_info{};
}

void init_render_backend(const ApplicationInfo &info) {
    if (s_application_info != nullptr) {
        ARS_LOG_ERROR("Can not init render backend twice");
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
        ARS_LOG_WARN("Backend is not init, but you want to destroy it.");
        return;
    }
    switch (s_application_info->backend) {
    case Backend::Vulkan:
        vk::destroy_vulkan_backend();
        break;
    }
    s_application_info.reset();
}

std::pair<std::unique_ptr<IContext>, std::unique_ptr<IWindow>>
IContext::create(WindowInfo *window_info) {
    if (s_application_info == nullptr) {
        ARS_LOG_ERROR("Render backend has not been initialized. Please call "
                      "init_render_backend first");
        return {};
    }

    std::unique_ptr<IContext> context{};
    std::unique_ptr<IWindow> window{};
    switch (s_application_info->backend) {
    case Backend::Vulkan:
        std::unique_ptr<vk::Swapchain> swapchain{};
        context = std::make_unique<vk::Context>(window_info, swapchain);
        window = std::move(swapchain);
        break;
    }
    return {std::move(context), std::move(window)};
}

std::shared_ptr<ITexture> IContext::create_texture(const TextureInfo &info) {
    // fix out of range inputs
    auto tex = info;
    tex.mip_levels =
        std::clamp(tex.mip_levels,
                   static_cast<uint32_t>(1),
                   calculate_mip_levels(tex.width, tex.height, tex.depth));

    return create_texture_impl(tex);
}
} // namespace ars::render