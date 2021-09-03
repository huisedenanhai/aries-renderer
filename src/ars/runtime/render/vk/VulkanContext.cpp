#include "VulkanContext.h"
#include "VulkanBuffer.h"
#include "VulkanMaterial.h"
#include "VulkanMesh.h"
#include "VulkanScene.h"
#include "VulkanSwapchain.h"
#include "VulkanTexture.h"
#include <GLFW/glfw3.h>
#include <algorithm>
#include <ars/runtime/core/Log.h>
#include <cassert>
#include <sstream>
#include <vector>

namespace ars::render {
namespace {
void remove_duplicates(std::vector<const char *> &vec) {
    std::sort(vec.begin(), vec.end(), [](const char *a, const char *b) {
        return strcmp(a, b) < 0;
    });
    vec.resize(std::distance(
        vec.begin(),
        std::unique(vec.begin(), vec.end(), [](const char *a, const char *b) {
            return strcmp(a, b) == 0;
        })));
}

VKAPI_ATTR VkBool32 VKAPI_CALL
debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
               [[maybe_unused]] VkDebugUtilsMessageTypeFlagsEXT message_type,
               const VkDebugUtilsMessengerCallbackDataEXT *p_callback_data,
               [[maybe_unused]] void *p_user_data) {
    std::stringstream ss;
    ss << "validation layer: " << p_callback_data->pMessage;

    // just ignore info and verbose log from vulkan
    if (message_severity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
        log_warn(ss.str());
    } else if (message_severity ==
               VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
        log_error(ss.str());
    }

    return VK_FALSE;
}

VkDebugUtilsMessengerCreateInfoEXT default_debug_utils_messenger_create_info() {
    VkDebugUtilsMessengerCreateInfoEXT create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    create_info.messageSeverity =
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                              VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                              VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    create_info.pfnUserCallback = debug_callback;
    create_info.pUserData = nullptr; // Optional
    return create_info;
}

const char *VALIDATION_LAYER = "VK_LAYER_KHRONOS_validation";

template <typename T, typename Getter>
bool check_contains(const char *const *target,
                    size_t target_count,
                    const T *available,
                    size_t available_count,
                    Getter &&getter) {
    for (int i = 0; i < target_count; i++) {
        bool exist = false;
        for (int j = 0; j < available_count; j++) {
            if (strcmp(getter(available[j]), target[i]) == 0) {
                exist = true;
                break;
            }
        }
        if (!exist) {
            return false;
        }
    }
    return true;
}

template <typename T, typename Getter>
inline bool check_contains(const std::vector<const char *> &target,
                           const std::vector<T> &available,
                           Getter &&getter) {
    return check_contains(&target[0],
                          target.size(),
                          &available[0],
                          available.size(),
                          std::forward<Getter>(getter));
}

std::vector<VkExtensionProperties> enumerate_instance_extensions() {
    uint32_t extension_count = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr);

    std::vector<VkExtensionProperties> available_extensions(extension_count);
    vkEnumerateInstanceExtensionProperties(
        nullptr, &extension_count, available_extensions.data());
    return available_extensions;
}

std::vector<const char *> get_instance_extensions(bool enable_validation,
                                                  bool need_presentation) {
    std::vector<const char *> enabled_extensions{};
    if (need_presentation) {
        uint32_t glfw_extension_count;
        const char **glfw_extensions;
        glfw_extensions =
            glfwGetRequiredInstanceExtensions(&glfw_extension_count);

        for (int i = 0; i < glfw_extension_count; i++) {
            enabled_extensions.push_back(glfw_extensions[i]);
        }
    }

    if (enable_validation) {
        enabled_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    auto available_extensions = enumerate_instance_extensions();
    // required by potentially enabled VK_KHR_portability_subset
    constexpr const char *PHYSICAL_DEVICE_PROPERTIES2 =
        "VK_KHR_get_physical_device_properties2";
    for (const auto &ext : available_extensions) {
        if (strcmp(ext.extensionName, PHYSICAL_DEVICE_PROPERTIES2) == 0) {
            enabled_extensions.push_back(PHYSICAL_DEVICE_PROPERTIES2);
            break;
        }
    }

    remove_duplicates(enabled_extensions);

    return enabled_extensions;
}

constexpr const char *PORTABILITY_SUBSET = "VK_KHR_portability_subset";

bool check_instance_extension_supported(
    const std::vector<const char *> &extensions) {
    return check_contains(extensions,
                          enumerate_instance_extensions(),
                          [](auto &&ext) { return ext.extensionName; });
}

bool check_validation_layers() {
    uint32_t layer_count;
    vkEnumerateInstanceLayerProperties(&layer_count, nullptr);

    std::vector<VkLayerProperties> available_layers(layer_count);
    vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());

    return check_contains({VALIDATION_LAYER},
                          available_layers,
                          [](auto &&layer) { return layer.layerName; });
}

std::unique_ptr<VulkanInstance>
create_vulkan_instance(const ApplicationInfo &app_info) {
    if (volkInitialize() != VK_SUCCESS) {
        panic("Failed to load vulkan function pointers");
    }

    bool enable_validation = app_info.enable_validation;

    if (enable_validation && !check_validation_layers()) {
        log_warn("validation layers requested but not available");
        enable_validation = false;
    }

    auto api_version = VK_API_VERSION_1_2;
    VkApplicationInfo vk_app_info{VK_STRUCTURE_TYPE_APPLICATION_INFO};
    vk_app_info.apiVersion = api_version;
    vk_app_info.pApplicationName = app_info.app_name.c_str();
    const auto &app_version = app_info.version;
    vk_app_info.applicationVersion = VK_MAKE_VERSION(
        app_version.major, app_version.minor, app_version.patch);
    vk_app_info.pEngineName = "Aries";
    vk_app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);

    auto extensions = get_instance_extensions(enable_validation,
                                              app_info.enable_presentation);
    if (!check_instance_extension_supported(extensions)) {
        panic("instance extension not supported");
    }

    std::vector<const char *> layers{};
    if (enable_validation) {
        layers.push_back(VALIDATION_LAYER);
    }

    VkInstanceCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    info.pApplicationInfo = &vk_app_info;
    info.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    info.ppEnabledExtensionNames = extensions.data();
    info.enabledLayerCount = static_cast<uint32_t>(layers.size());
    info.ppEnabledLayerNames = layers.data();

    VkDebugUtilsMessengerCreateInfoEXT debug_info{};
    if (enable_validation) {
        debug_info = default_debug_utils_messenger_create_info();
        info.pNext = (VkDebugUtilsMessengerCreateInfoEXT *)&debug_info;
    }

    VkInstance instance{};
    VkAllocationCallbacks *allocator = nullptr;
    if (vkCreateInstance(&info, allocator, &instance) != VK_SUCCESS) {
        panic("Failed to create vulkan instance");
    }

    return std::make_unique<VulkanInstance>(instance, api_version, allocator);
}

struct VulkanEnvironment {
    std::unique_ptr<VulkanInstance> instance = nullptr;
    VkDebugUtilsMessengerEXT debug_messenger = VK_NULL_HANDLE;

    VulkanEnvironment(std::unique_ptr<VulkanInstance> ins,
                      VkDebugUtilsMessengerEXT debug)
        : instance(std::move(ins)), debug_messenger(debug) {}

    ~VulkanEnvironment() {
        if (!instance) {
            return;
        }
        if (debug_messenger != VK_NULL_HANDLE) {
            instance->Destroy(debug_messenger);
        }
        instance.reset();
    }
};

VkDebugUtilsMessengerEXT create_debug_messenger(VulkanInstance *instance) {
    VkDebugUtilsMessengerEXT debug_messenger;
    auto info = default_debug_utils_messenger_create_info();
    if (instance->Create(&info, &debug_messenger) != VK_SUCCESS) {
        log_error("Failed to create debug messenger");
        return VK_NULL_HANDLE;
    }
    return debug_messenger;
}

std::unique_ptr<VulkanEnvironment> s_vulkan{};

} // namespace

void init_vulkan_backend(const ApplicationInfo &app_info) {
    assert(s_vulkan == nullptr);
    auto instance = create_vulkan_instance(app_info);
    VkDebugUtilsMessengerEXT debug_messenger = VK_NULL_HANDLE;
    if (app_info.enable_validation) {
        debug_messenger = create_debug_messenger(instance.get());
    }
    s_vulkan = std::make_unique<VulkanEnvironment>(std::move(instance),
                                                   debug_messenger);
}

void destroy_vulkan_backend() {
    assert(s_vulkan != nullptr);
    s_vulkan.reset();
}

VulkanContext::VulkanContext() {}

std::unique_ptr<ISwapchain>
VulkanContext::create_swapchain(GLFWwindow *window) {
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

VulkanContext::~VulkanContext() = default;
} // namespace ars::render
