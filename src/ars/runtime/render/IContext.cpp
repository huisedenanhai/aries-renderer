#include "IContext.h"
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

std::unique_ptr<IContext> IContext::create(GLFWwindow *window) {
    if (s_application_info == nullptr) {
        log_error("Render backend has not been initialized. Please call "
                  "init_render_backend first");
        return nullptr;
    }

    std::unique_ptr<IContext> result{};
    switch (s_application_info->backend) {
    case Backend::Vulkan:
        result = std::make_unique<vk::Context>(window);
        break;
    }
    return result;
}

std::unique_ptr<ITexture> IContext::create_texture_2d(Format format,
                                                      uint32_t width,
                                                      uint32_t height,
                                                      uint32_t mip_levels) {
    TextureInfo info{};
    info.format = format;
    info.type = TextureType::Texture2D;
    info.width = width;
    info.height = height;
    info.depth = 1;
    info.mip_levels = mip_levels;
    info.array_layers = 1;
    return create_texture(info);
}

std::unique_ptr<ITexture> IContext::create_texture(const TextureInfo &info) {
    // fix out of range inputs
    auto tex = info;
    tex.mip_levels =
        std::clamp(tex.mip_levels,
                   static_cast<uint32_t>(1),
                   calculate_mip_levels(tex.width, tex.height, tex.depth));

    return create_texture_impl(tex);
}
} // namespace ars::render