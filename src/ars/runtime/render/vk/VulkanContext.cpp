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
#include <set>
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
create_vulkan_instance(const ApplicationInfo &app_info,
                       bool enable_validation) {

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

    return std::make_unique<VulkanInstance>(
        instance, api_version, app_info.enable_presentation, allocator);
}

struct VulkanEnvironment {
    std::unique_ptr<VulkanInstance> instance = nullptr;
    VkDebugUtilsMessengerEXT debug_messenger = VK_NULL_HANDLE;

    VulkanEnvironment(std::unique_ptr<VulkanInstance> ins,
                      VkDebugUtilsMessengerEXT debug)
        : instance(std::move(ins)), debug_messenger(debug) {}

    [[nodiscard]] bool validation_enabled() const {
        return debug_messenger != VK_NULL_HANDLE;
    }

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

    if (volkInitialize() != VK_SUCCESS) {
        panic("Failed to load vulkan function pointers");
    }

    bool enable_validation = app_info.enable_validation;
    if (enable_validation && !check_validation_layers()) {
        log_warn("Validation layers requested but not available");
        enable_validation = false;
    }

    auto instance = create_vulkan_instance(app_info, enable_validation);
    VkDebugUtilsMessengerEXT debug_messenger = VK_NULL_HANDLE;
    if (enable_validation) {
        debug_messenger = create_debug_messenger(instance.get());
    }
    s_vulkan = std::make_unique<VulkanEnvironment>(std::move(instance),
                                                   debug_messenger);
}

void destroy_vulkan_backend() {
    assert(s_vulkan != nullptr);
    s_vulkan.reset();
}

std::unique_ptr<ISwapchain>
VulkanContext::create_swapchain(GLFWwindow *window) {
    if (window == _cached_window && _cached_swapchain != nullptr) {
        _cached_window = nullptr;
        return std::move(_cached_swapchain);
    }

    return nullptr;
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

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities{};
    std::vector<VkSurfaceFormatKHR> formats{};
    std::vector<VkPresentModeKHR> present_modes{};
};

SwapChainSupportDetails
query_swapchain_support(VulkanInstance *instance,
                        VkPhysicalDevice physical_device,
                        VkSurfaceKHR surface) {
    SwapChainSupportDetails details;
    instance->GetPhysicalDeviceSurfaceCapabilitiesKHR(
        physical_device, surface, &details.capabilities);

    uint32_t format_count = 0;
    instance->GetPhysicalDeviceSurfaceFormatsKHR(
        physical_device, surface, &format_count, nullptr);

    if (format_count != 0) {
        details.formats.resize(format_count);
        instance->GetPhysicalDeviceSurfaceFormatsKHR(
            physical_device, surface, &format_count, details.formats.data());
    }

    uint32_t present_mode_count = 0;
    instance->GetPhysicalDeviceSurfacePresentModesKHR(
        physical_device, surface, &present_mode_count, nullptr);

    if (present_mode_count != 0) {
        details.present_modes.resize(present_mode_count);
        instance->GetPhysicalDeviceSurfacePresentModesKHR(
            physical_device,
            surface,
            &present_mode_count,
            details.present_modes.data());
    }

    return details;
}

namespace {
std::vector<VkExtensionProperties>
enumerate_device_extensions(VulkanInstance *instance,
                            VkPhysicalDevice physical_device) {
    uint32_t available_extension_count = 0;
    instance->EnumerateDeviceExtensionProperties(
        physical_device, nullptr, &available_extension_count, nullptr);
    std::vector<VkExtensionProperties> available_extensions(
        available_extension_count);
    instance->EnumerateDeviceExtensionProperties(physical_device,
                                                 nullptr,
                                                 &available_extension_count,
                                                 available_extensions.data());
    return available_extensions;
}

bool is_portability_subset(VulkanInstance *instance,
                           VkPhysicalDevice physical_device) {
    auto available_extensions =
        enumerate_device_extensions(instance, physical_device);
    for (const auto &ext : available_extensions) {
        if (strcmp(ext.extensionName, PORTABILITY_SUBSET) == 0) {
            return true;
        }
    }
    return false;
}

bool check_device_extension_supported(
    VulkanInstance *instance,
    VkPhysicalDevice physical_device,
    const std::vector<const char *> &extensions) {
    return check_contains(
        extensions,
        enumerate_device_extensions(instance, physical_device),
        [](auto &&ext) { return ext.extensionName; });
}

std::vector<const char *>
get_device_extensions(VulkanInstance *instance,
                      VkPhysicalDevice physical_device,
                      bool need_swapchain) {
    auto available_extensions =
        enumerate_device_extensions(instance, physical_device);
    std::vector<const char *> enabled_extensions{};

    if (need_swapchain) {
        enabled_extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    }

    for (const auto &ext : available_extensions) {
        // must include VK_KHR_portability_subset if available
        if (strcmp(ext.extensionName, PORTABILITY_SUBSET) == 0) {
            enabled_extensions.push_back(PORTABILITY_SUBSET);
            break;
        }
    }

    remove_duplicates(enabled_extensions);

    return enabled_extensions;
}

struct QueueFamilyIndices {
    std::optional<uint32_t> graphics_family;
    std::optional<uint32_t> present_family;

    [[nodiscard]] bool is_complete() const {
        return graphics_family.has_value() && present_family.has_value();
    }
};

QueueFamilyIndices find_queue_families(VulkanInstance *instance,
                                       VkPhysicalDevice device,
                                       VkSurfaceKHR surface) {
    QueueFamilyIndices indices;
    uint32_t queue_family_count = 0;
    instance->GetPhysicalDeviceQueueFamilyProperties(
        device, &queue_family_count, nullptr);

    std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
    instance->GetPhysicalDeviceQueueFamilyProperties(
        device, &queue_family_count, queue_families.data());

    for (int i = 0; i < queue_families.size(); i++) {
        const auto &queue_family = queue_families[i];
        if (queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphics_family = i;
        }

        VkBool32 present_support = false;
        if (surface != VK_NULL_HANDLE) {
            instance->GetPhysicalDeviceSurfaceSupportKHR(
                device, i, surface, &present_support);
        } else {
            present_support = true;
        }
        if (present_support) {
            indices.present_family = i;
        }

        if (indices.is_complete()) {
            break;
        }
    }

    return indices;
}

// return 0 to reject
// surface is VK_NULL_HANDLE if swapchain support is required
int rate_device_suitability(VulkanInstance *instance,
                            VkPhysicalDevice physical_device,
                            VkSurfaceKHR surface) {
    assert(physical_device != VK_NULL_HANDLE);
    VkPhysicalDeviceProperties properties;
    instance->GetPhysicalDeviceProperties(physical_device, &properties);

    bool need_swapchain = surface != VK_NULL_HANDLE;

    if (!check_device_extension_supported(
            instance,
            physical_device,
            get_device_extensions(instance, physical_device, need_swapchain))) {
        return 0;
    }

    if (!find_queue_families(instance, physical_device, surface)
             .is_complete()) {
        return 0;
    }

    if (need_swapchain) {
        auto swapchain_support =
            query_swapchain_support(instance, physical_device, surface);
        if (swapchain_support.formats.empty() ||
            swapchain_support.present_modes.empty()) {
            return 0;
        }
    }

    int score = 0;
    // Discrete GPUs have a significant performance advantage
    if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
        score += 1000;
    }

    // prefer native vulkan driver, rather than a wrapper around other apis
    if (!is_portability_subset(instance, physical_device)) {
        score += 100;
    }

    // Maximum possible size of textures affects graphics quality
    score += static_cast<int>(properties.limits.maxImageDimension2D);

    return score;
}

VkPhysicalDevice choose_physical_device(VulkanInstance *instance,
                                        VkSurfaceKHR surface) {
    uint32_t device_count = 0;
    instance->EnumeratePhysicalDevices(&device_count, nullptr);
    if (device_count == 0) {
        panic("Failed to find GPUs with Vulkan support!");
    }

    std::vector<VkPhysicalDevice> devices(device_count);
    instance->EnumeratePhysicalDevices(&device_count, devices.data());

    VkPhysicalDevice candidate = VK_NULL_HANDLE;
    int maxScore = 0;
    for (const auto &device : devices) {
        int score = rate_device_suitability(instance, device, surface);
        if (score > maxScore) {
            candidate = device;
        }
    }

    if (candidate == VK_NULL_HANDLE) {
        panic("Failed to find a suitable GPU!");
    }

    return candidate;
}
} // namespace

void VulkanContext::init_device_and_queues(VulkanInstance *instance,
                                           bool enable_validation,
                                           VkSurfaceKHR surface) {
    auto physical_device = choose_physical_device(instance, surface);
    QueueFamilyIndices indices =
        find_queue_families(instance, physical_device, surface);

    assert(indices.is_complete());

    std::set<uint32_t> unique_queue_families = {indices.graphics_family.value(),
                                                indices.present_family.value()};
    std::vector<VkDeviceQueueCreateInfo> queue_create_infos{};
    queue_create_infos.reserve(unique_queue_families.size());

    float queuePriority = 1.0f;
    for (auto &family : unique_queue_families) {
        VkDeviceQueueCreateInfo queue_create_info{};
        queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_create_info.queueFamilyIndex = family;
        queue_create_info.queueCount = 1;
        queue_create_info.pQueuePriorities = &queuePriority;

        queue_create_infos.push_back(queue_create_info);
    }

    VkPhysicalDeviceFeatures device_features{};

    VkDeviceCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    create_info.pQueueCreateInfos = queue_create_infos.data();
    create_info.queueCreateInfoCount = (uint32_t)queue_create_infos.size();
    create_info.pEnabledFeatures = &device_features;

    auto enabled_extensions = get_device_extensions(
        instance, physical_device, surface != VK_NULL_HANDLE);
    create_info.enabledExtensionCount = (uint32_t)enabled_extensions.size();
    create_info.ppEnabledExtensionNames = enabled_extensions.data();

    if (enable_validation) {
        create_info.enabledLayerCount = 1;
        create_info.ppEnabledLayerNames = &VALIDATION_LAYER;
    } else {
        create_info.enabledLayerCount = 0;
    }

    VkDevice device;
    if (instance->Create(physical_device, &create_info, &device) !=
        VK_SUCCESS) {
        panic("failed to create logical device!");
    }

    _device = std::make_unique<VulkanDevice>(instance, device, physical_device);

    _device->GetDeviceQueue(
        indices.graphics_family.value(), 0, &_graphics_queue.queue);
    _graphics_queue.family_index = indices.graphics_family.value();
    _device->GetDeviceQueue(
        indices.present_family.value(), 0, &_present_queue.queue);
    _present_queue.family_index = indices.present_family.value();
}

VulkanContext::VulkanContext(GLFWwindow *window) {
    VkSurfaceKHR surface = VK_NULL_HANDLE;
    if (window != nullptr) {
        if (!s_vulkan->instance->presentation_enabled()) {
            log_error("Presentation is not enabled for the render backend, but "
                      "you still supply a window for presentation");
        } else if (glfwCreateWindowSurface(s_vulkan->instance->instance(),
                                           window,
                                           s_vulkan->instance->allocator(),
                                           &surface) != VK_SUCCESS) {
            log_error("Can not create surface for the window");
        }
    }

    init_device_and_queues(
        s_vulkan->instance.get(), s_vulkan->validation_enabled(), surface);

    if (surface != VK_NULL_HANDLE) {
        _cached_window = window;
        _cached_swapchain = std::make_unique<VulkanSwapchain>(this, surface);
    }

    _vma = std::make_unique<VulkanMemoryAllocator>(_device.get());
}

VulkanInstance *VulkanContext::get_instance() const {
    return _device->instance();
}

VulkanMemoryAllocator *VulkanContext::get_vma() const {
    return _vma.get();
}

VulkanContext::~VulkanContext() = default;
} // namespace ars::render
