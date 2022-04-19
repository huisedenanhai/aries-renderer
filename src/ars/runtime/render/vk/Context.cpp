#include "Context.h"
#include "Lut.h"
#include "Material.h"
#include "Mesh.h"
#include "Profiler.h"
#include "Scene.h"
#include "Sky.h"
#include "Swapchain.h"
#include "features/Renderer.h"
#include <GLFW/glfw3.h>
#include <algorithm>
#include <ars/runtime/core/Log.h>
#include <cassert>
#include <sstream>
#include <vector>

namespace ars::render::vk {
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
        ARS_LOG_WARN(ss.str());
    } else if (message_severity ==
               VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
        ARS_LOG_ERROR(ss.str());
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
    return check_contains(target.data(),
                          target.size(),
                          available.data(),
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

std::unique_ptr<Instance>
create_vulkan_instance(const ApplicationInfo &app_info,
                       bool enable_validation) {
    auto api_version = VK_API_VERSION_1_1;
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
        ARS_LOG_CRITICAL("Instance extension not supported");
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
        ARS_LOG_CRITICAL("Failed to create vulkan instance");
    }

    return std::make_unique<Instance>(
        instance, api_version, app_info.enable_presentation, allocator);
}

struct VulkanEnvironment {
    std::unique_ptr<Instance> instance = nullptr;
    VkDebugUtilsMessengerEXT debug_messenger = VK_NULL_HANDLE;
    bool profiler_enabled = false;

    VulkanEnvironment(std::unique_ptr<Instance> ins,
                      VkDebugUtilsMessengerEXT debug,
                      bool profiler_enabled)
        : instance(std::move(ins)), debug_messenger(debug),
          profiler_enabled(profiler_enabled) {}

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

VkDebugUtilsMessengerEXT create_debug_messenger(Instance *instance) {
    VkDebugUtilsMessengerEXT debug_messenger;
    auto info = default_debug_utils_messenger_create_info();
    if (instance->Create(&info, &debug_messenger) != VK_SUCCESS) {
        ARS_LOG_ERROR("Failed to create debug messenger");
        return VK_NULL_HANDLE;
    }
    return debug_messenger;
}

std::unique_ptr<VulkanEnvironment> s_vulkan{};

} // namespace

void init_vulkan_backend(const ApplicationInfo &app_info) {
    assert(s_vulkan == nullptr);

    glfwInit();

    if (volkInitialize() != VK_SUCCESS) {
        ARS_LOG_CRITICAL("Failed to load vulkan function pointers");
    }

    bool enable_validation = app_info.enable_validation;
    if (enable_validation && !check_validation_layers()) {
        ARS_LOG_WARN("Validation layers requested but not available");
        enable_validation = false;
    }

    auto instance = create_vulkan_instance(app_info, enable_validation);
    VkDebugUtilsMessengerEXT debug_messenger = VK_NULL_HANDLE;
    if (enable_validation) {
        debug_messenger = create_debug_messenger(instance.get());
    }
    s_vulkan = std::make_unique<VulkanEnvironment>(
        std::move(instance), debug_messenger, app_info.enable_profiler);
}

void destroy_vulkan_backend() {
    assert(s_vulkan != nullptr);

    s_vulkan.reset();

    glfwTerminate();
}

std::unique_ptr<IWindow> Context::create_window(const WindowInfo &info) {
    auto [window, surface] = create_window_and_surface(&info);
    return std::make_unique<Swapchain>(this, surface, window, true);
}

std::unique_ptr<IScene> Context::create_scene() {
    return std::make_unique<Scene>(this);
}

std::shared_ptr<IMesh> Context::create_mesh(const MeshInfo &info) {
    return std::make_shared<Mesh>(this, info);
}

namespace {
std::vector<VkExtensionProperties>
enumerate_device_extensions(Instance *instance,
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

bool is_portability_subset(Instance *instance,
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
    Instance *instance,
    VkPhysicalDevice physical_device,
    const std::vector<const char *> &extensions) {
    return check_contains(
        extensions,
        enumerate_device_extensions(instance, physical_device),
        [](auto &&ext) { return ext.extensionName; });
}

std::vector<const char *> get_device_extensions(
    Instance *instance, VkPhysicalDevice physical_device, bool need_swapchain) {
    auto available_extensions =
        enumerate_device_extensions(instance, physical_device);
    std::vector<const char *> enabled_extensions{"VK_EXT_descriptor_indexing"};

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
    // this queue family supports all operations we need
    std::optional<uint32_t> primary_family;

    [[nodiscard]] bool is_complete() const {
        return primary_family.has_value();
    }
};

QueueFamilyIndices find_queue_families(Instance *instance,
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
        bool supports_graphics =
            queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT;
        bool supports_compute = queue_family.queueFlags & VK_QUEUE_COMPUTE_BIT;

        bool valid = supports_graphics && supports_compute;

        if (surface != VK_NULL_HANDLE) {
            VkBool32 support_present = false;
            instance->GetPhysicalDeviceSurfaceSupportKHR(
                device, i, surface, &support_present);
            valid = valid && support_present;
        }

        if (valid) {
            indices.primary_family = i;
        }

        if (indices.is_complete()) {
            break;
        }
    }

    return indices;
}

// return 0 to reject
// surface is VK_NULL_HANDLE if swapchain support is required
int rate_device_suitability(Instance *instance,
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

    // Prefer native vulkan driver, rather than a wrapper around other apis
    if (!is_portability_subset(instance, physical_device)) {
        score += 100;
    }

    // Maximum possible size of textures affects graphics quality
    score += static_cast<int>(properties.limits.maxImageDimension2D);

    // Check device features
    VkPhysicalDeviceFeatures features{};
    instance->GetPhysicalDeviceFeatures(physical_device, &features);
    if (features.samplerAnisotropy == VK_TRUE) {
        score += 100;
    }

    return score;
}

VkPhysicalDevice choose_physical_device(Instance *instance,
                                        VkSurfaceKHR surface) {
    uint32_t device_count = 0;
    instance->EnumeratePhysicalDevices(&device_count, nullptr);
    if (device_count == 0) {
        ARS_LOG_CRITICAL("Failed to find GPUs with Vulkan support!");
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
        ARS_LOG_CRITICAL("Failed to find a suitable GPU!");
    }

    return candidate;
}

std::string dump_version(uint32_t version) {
    return fmt::format("{}.{}.{}",
                       VK_VERSION_MAJOR(version),
                       VK_VERSION_MINOR(version),
                       VK_VERSION_PATCH(version));
}
} // namespace

void Context::init_device_and_queues(Instance *instance,
                                     bool enable_validation,
                                     VkSurfaceKHR surface) {
    auto physical_device = choose_physical_device(instance, surface);
    QueueFamilyIndices indices =
        find_queue_families(instance, physical_device, surface);

    assert(indices.is_complete());

    VkDeviceQueueCreateInfo queue_create_info{};

    float queue_priority = 1.0f;
    queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_create_info.queueFamilyIndex = indices.primary_family.value();
    queue_create_info.queueCount = 1;
    queue_create_info.pQueuePriorities = &queue_priority;

    VkPhysicalDeviceFeatures2 device_features2{
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2};
    VkPhysicalDeviceVulkan12Features vk12_features{
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES};
    VkPhysicalDeviceDescriptorIndexingFeatures desc_indexing_features{
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES};
    device_features2.pNext = &vk12_features;
    vk12_features.pNext = &desc_indexing_features;

    instance->GetPhysicalDeviceFeatures2(physical_device, &device_features2);

    VkPhysicalDeviceFeatures device_features = device_features2.features;

    VkPhysicalDeviceFeatures enabled_features{};

    // Enable sampler anisotropy if the device supports
    enabled_features.samplerAnisotropy = device_features.samplerAnisotropy;

    VkDeviceCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    create_info.pQueueCreateInfos = &queue_create_info;
    create_info.queueCreateInfoCount = 1;
    create_info.pEnabledFeatures = &enabled_features;

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
        ARS_LOG_CRITICAL("failed to create logical device!");
    }

    _device = std::make_unique<Device>(instance, device, physical_device);
    _queue = std::make_unique<Queue>(this, indices.primary_family.value());

    VkPhysicalDeviceProperties device_properties{};
    instance->GetPhysicalDeviceProperties(physical_device, &device_properties);

    // Init context properties
    if (enabled_features.samplerAnisotropy == VK_TRUE) {
        _properties.anisotropic_sampler_enabled = true;
        _properties.max_sampler_anisotropy =
            device_properties.limits.maxSamplerAnisotropy;
    }
    _properties.time_stamp_compute_graphics =
        device_properties.limits.timestampComputeAndGraphics;
    _properties.time_stamp_period_ns = device_properties.limits.timestampPeriod;

    // Log info
    std::stringstream ss;
    ss << "Vulkan Physical Device" << std::endl;
    ss << "name: " << device_properties.deviceName << std::endl;
    ss << "api version: " << dump_version(device_properties.apiVersion)
       << std::endl;
    ss << "driver version: " << device_properties.driverVersion << std::endl;
    ss << "enabled extensions: " << std::endl;
    for (auto &ext : enabled_extensions) {
        ss << "\t" << ext << std::endl;
    }
    ss << "anisotropic sampler enabled: "
       << _properties.anisotropic_sampler_enabled << std::endl;
    ss << "max sampler anisotropy: " << _properties.max_sampler_anisotropy
       << std::endl;
    ss << "time stamp period ns: " << _properties.time_stamp_period_ns
       << std::endl;

    ARS_LOG_INFO(ss.str());
}

Context::Context(const WindowInfo *info,
                 std::unique_ptr<Swapchain> &swapchain) {
    auto [window, surface] = create_window_and_surface(info);

    _vma = std::make_unique<VulkanMemoryAllocator>(_device.get());
    init_command_pool();
    init_pipeline_cache();
    init_descriptor_arena();

    // Now context ready to create the swapchain
    if (window != nullptr && surface != VK_NULL_HANDLE) {
        swapchain = std::make_unique<Swapchain>(this, surface, window, true);
    }

    init_default_textures();
    _lut = std::make_unique<Lut>(this);
    _ibl = std::make_unique<ImageBasedLighting>(this);
    _renderer_data = std::make_unique<RendererContextData>(this);
    // Renderer data is required for material initialization
    _material_factory = std::make_unique<MaterialFactory>(this);
    Context::init_profiler();
}

Instance *Context::instance() const {
    return _device->instance();
}

VulkanMemoryAllocator *Context::vma() const {
    return _vma.get();
}

Device *Context::device() const {
    return _device.get();
}

Queue *Context::queue() const {
    return _queue.get();
}

std::tuple<GLFWwindow *, VkSurfaceKHR>
Context::create_window_and_surface(const WindowInfo *info) {
    GLFWwindow *window = nullptr;

    if (info != nullptr) {
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        auto logical_width = info->logical_size.width;
        auto logical_height = info->logical_size.height;
        int monitor_x, monitor_y, monitor_width, monitor_height;
        glfwGetMonitorWorkarea(glfwGetPrimaryMonitor(),
                               &monitor_x,
                               &monitor_y,
                               &monitor_width,
                               &monitor_height);
        if (logical_width == 0) {
            logical_width = monitor_width;
        }
        if (logical_height == 0) {
            logical_height = monitor_height;
        }
        window = glfwCreateWindow(static_cast<int>(logical_width),
                                  static_cast<int>(logical_height),
                                  info->title.c_str(),
                                  nullptr,
                                  nullptr);
    }

    return {window, create_surface(window)};
}

std::shared_ptr<ITexture>
Context::create_texture_impl(const TextureInfo &info) {
    auto tex = create_texture(translate(info));
    return std::make_shared<TextureAdapter>(info, std::move(tex));
}

bool Context::begin_frame() {
    glfwPollEvents();
    _queue->flush();
    _device->ResetCommandPool(_command_pool, 0);
    _descriptor_arena->reset();
    gc();

    if (_profiler != nullptr) {
        _profiler->flush();
        _profiler->begin_frame();
    }
    return true;
}

void Context::end_frame() {
    for (auto swapchain : _registered_swapchains) {
        swapchain->on_frame_ends();
    }
}

Context::~Context() {
    _queue->flush();
    _material_factory.reset();
    gc();
    if (_pipeline_cache != VK_NULL_HANDLE) {
        _device->Destroy(_pipeline_cache);
    }

    if (_command_pool != VK_NULL_HANDLE) {
        _device->Destroy(_command_pool);
    }
}

Queue::Queue(Context *context, uint32_t family_index)
    : _context(context), _family_index(family_index) {
    auto instance = _context->instance();
    auto device = _context->device();
    device->GetDeviceQueue(family_index, 0, &_queue);

    uint32_t property_count = 0;
    instance->GetPhysicalDeviceQueueFamilyProperties(
        device->physical_device(), &property_count, nullptr);

    std::vector<VkQueueFamilyProperties> properties{};
    properties.resize(property_count);
    instance->GetPhysicalDeviceQueueFamilyProperties(
        device->physical_device(), &property_count, properties.data());

    assert(family_index < property_count);
    _family_properties = properties[family_index];
}

uint32_t Queue::family_index() const {
    return _family_index;
}

VkQueue Queue::queue() const {
    return _queue;
}

void Queue::submit(CommandBuffer *command_buffer,
                   uint32_t wait_semaphore_count,
                   VkSemaphore *wait_semaphores) {
    assert(command_buffer != nullptr);
    auto device = _context->device();

    VkSubmitInfo info{VK_STRUCTURE_TYPE_SUBMIT_INFO};
    std::vector<VkSemaphore> wait_sems{};
    wait_sems.reserve(1 + wait_semaphore_count);
    if (!_semaphores.empty()) {
        wait_sems.push_back(_semaphores.back());
    }

    for (int i = 0; i < wait_semaphore_count; i++) {
        wait_sems.push_back(wait_semaphores[i]);
    }

    std::vector<VkPipelineStageFlags> dst_masks(
        wait_sems.size(), VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT);

    info.waitSemaphoreCount = static_cast<uint32_t>(wait_sems.size());
    info.pWaitSemaphores = wait_sems.data();
    info.pWaitDstStageMask = dst_masks.data();

    VkSemaphoreCreateInfo sem_info{VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
    VkSemaphore signal_sem = VK_NULL_HANDLE;

    if (device->Create(&sem_info, &signal_sem) != VK_SUCCESS) {
        ARS_LOG_CRITICAL("Failed to create semaphore");
    }

    _semaphores.push_back(signal_sem);

    info.signalSemaphoreCount = 1;
    info.pSignalSemaphores = &signal_sem;

    info.commandBufferCount = 1;
    auto cmd = command_buffer->command_buffer();
    info.pCommandBuffers = &cmd;

    if (device->QueueSubmit(_queue, 1, &info, VK_NULL_HANDLE) !=
        VK_NULL_HANDLE) {
        ARS_LOG_CRITICAL("Failed to submit command to queue");
    }
}

Queue::~Queue() {
    flush();
}

void Queue::flush() {
    auto device = _context->device();
    device->QueueWaitIdle(_queue);
    for (auto sem : _semaphores) {
        device->Destroy(sem);
    }
    _semaphores.clear();
}

VkSemaphore Queue::get_semaphore() const {
    if (!_semaphores.empty()) {
        return _semaphores.back();
    }
    return VK_NULL_HANDLE;
}

VkQueueFamilyProperties Queue::family_properties() const {
    return _family_properties;
}

void Context::init_command_pool() {
    VkCommandPoolCreateInfo info{VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
    info.queueFamilyIndex = _queue->family_index();
    if (_device->Create(&info, &_command_pool) != VK_SUCCESS) {
        ARS_LOG_CRITICAL("Can not create command pool");
    }
}

template <typename T, typename... Args>
Handle<T> Context::create_handle(std::vector<std::shared_ptr<T>> &pool,
                                 Args &&...args) {
    auto res = std::make_shared<T>(std::forward<Args>(args)...);
    pool.push_back(res);
    return Handle<T>(res);
}

Handle<CommandBuffer>
Context::create_command_buffer(VkCommandBufferLevel level) {
    return create_handle(_command_buffers, this, _command_pool, level);
}

Handle<Texture> Context::create_texture(const TextureCreateInfo &info) {
    return create_handle(_textures, this, info);
}

Handle<Buffer> Context::create_buffer(VkDeviceSize size,
                                      VkBufferUsageFlags buffer_usage,
                                      VmaMemoryUsage memory_usage) {
    return create_handle(_buffers, this, size, buffer_usage, memory_usage);
}

namespace {
template <typename T> void run_gc(std::vector<std::shared_ptr<T>> &pool) {
    pool.erase(std::remove_if(pool.begin(),
                              pool.end(),
                              [](auto &res) { return res.use_count() <= 1; }),
               pool.end());
}
} // namespace

void Context::gc() {
    run_gc(_textures);
    run_gc(_command_buffers);
    run_gc(_buffers);

    _tmp_framebuffers.clear();
}

VkPipelineCache Context::pipeline_cache() const {
    return _pipeline_cache;
}

void Context::init_pipeline_cache() {
    VkPipelineCacheCreateInfo info{
        VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO};

    if (_device->Create(&info, &_pipeline_cache) != VK_SUCCESS) {
        ARS_LOG_ERROR(
            "Failed to create pipeline cache, pipeline creation might be slow");
    }
}

DescriptorArena *Context::descriptor_arena() const {
    return _descriptor_arena.get();
}

void Context::init_descriptor_arena() {
    uint32_t count = 255;

    std::vector<VkDescriptorPoolSize> pool_sizes{
        {VK_DESCRIPTOR_TYPE_SAMPLER, count},
        {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, count},
        {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, count},
        {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, count},
        {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, count},
        {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, count},
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, count},
        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, count},
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, count},
        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, count},
        {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, count}};

    VkPhysicalDeviceProperties properties;
    _device->instance()->GetPhysicalDeviceProperties(_device->physical_device(),
                                                     &properties);

    _descriptor_arena = std::make_unique<DescriptorArena>(
        _device.get(), pool_sizes, properties.limits.maxBoundDescriptorSets);
}

const ContextProperties &Context::properties() const {
    return _properties;
}

VkSurfaceKHR Context::create_surface(GLFWwindow *window) {
    VkSurfaceKHR surface = VK_NULL_HANDLE;
    if (window != nullptr) {
        if (!s_vulkan->instance->presentation_enabled()) {
            ARS_LOG_ERROR(
                "Presentation is not enabled for the render backend, but "
                "you still supply a window for presentation");
        } else if (glfwCreateWindowSurface(s_vulkan->instance->instance(),
                                           window,
                                           s_vulkan->instance->allocator(),
                                           &surface) != VK_SUCCESS) {
            ARS_LOG_ERROR("Can not create surface for the window");
        }
    }

    if (_device == nullptr) {
        init_device_and_queues(
            s_vulkan->instance.get(), s_vulkan->validation_enabled(), surface);
    }

    if (surface != VK_NULL_HANDLE) {
        // must check if the surface is supported by the physical device
        if (VkBool32 is_supported_surface = VK_FALSE;
            instance()->GetPhysicalDeviceSurfaceSupportKHR(
                _device->physical_device(),
                _queue->family_index(),
                surface,
                &is_supported_surface) != VK_SUCCESS ||
            is_supported_surface != VK_TRUE) {

            ARS_LOG_ERROR(
                "The physical device of the context does not support the "
                "presentation of the given window");

            // cleanup
            instance()->Destroy(surface);
            return VK_NULL_HANDLE;
        }
    }

    return surface;
}

std::unique_ptr<Swapchain> Context::create_swapchain(GLFWwindow *window,
                                                     bool owns_window) {
    auto surface = create_surface(window);
    return std::make_unique<Swapchain>(this, surface, window, owns_window);
}

Framebuffer *
Context::create_tmp_framebuffer(RenderPass *render_pass,
                                std::vector<Handle<Texture>> attachments) {
    auto framebuffer =
        std::make_unique<Framebuffer>(render_pass, std::move(attachments));
    _tmp_framebuffers.emplace_back(std::move(framebuffer));
    return _tmp_framebuffers.back().get();
}

void Context::register_swapchain(Swapchain *swapchain) {
    _registered_swapchains.insert(swapchain);
}

void Context::unregister_swapchain(Swapchain *swapchain) {
    _registered_swapchains.erase(swapchain);
}

std::shared_ptr<IMaterial> Context::create_material(MaterialType type) {
    return _material_factory->create_material(type);
}

std::shared_ptr<ITexture> Context::default_texture(DefaultTexture tex) {
    assert(tex != DefaultTexture::Count);
    return _default_textures[static_cast<uint32_t>(tex)];
}

std::shared_ptr<ITexture>
Context::create_single_color_texture_2d(glm::vec4 color) {
    TextureInfo info{};
    info.format = Format::R32G32B32A32_SFLOAT;
    info.width = 1;
    info.height = 1;
    info.depth = 1;
    info.mip_levels = 1;

    auto tex = IContext::create_texture(info);
    tex->set_data(&color, sizeof(glm::vec4), 0, 0, 0, 0, 0, 1, 1, 1);
    return tex;
}

void Context::init_default_textures() {
    _default_textures[static_cast<uint32_t>(DefaultTexture::White)] =
        create_single_color_texture_2d({1.0f, 1.0f, 1.0f, 1.0f});
    _default_textures[static_cast<uint32_t>(DefaultTexture::Zero)] =
        create_single_color_texture_2d({0.0f, 0.0f, 0.0f, 0.0f});
    _default_textures[static_cast<uint32_t>(DefaultTexture::Normal)] =
        create_single_color_texture_2d({0.5f, 0.5f, 1.0f, 1.0f});
    _default_textures[static_cast<uint32_t>(DefaultTexture::WhiteCubeMap)] =
        create_single_color_cube_map({1.0f, 1.0f, 1.0f, 1.0f});
}

Lut *Context::lut() const {
    return _lut.get();
}

std::shared_ptr<ITexture>
Context::create_single_color_cube_map(glm::vec4 color) {
    TextureInfo info{};
    info.type = TextureType::CubeMap;
    info.format = Format::R32G32B32A32_SFLOAT;
    info.width = 1;
    info.height = 1;
    info.depth = 1;
    info.mip_levels = 1;

    auto tex = IContext::create_texture(info);
    for (int i = 0; i < 6; i++) {
        tex->set_data(&color, sizeof(glm::vec4), 0, i, 0, 0, 0, 1, 1, 1);
    }
    return tex;
}

Profiler *Context::profiler() const {
    return _profiler.get();
}

void Context::init_profiler() {
    if (s_vulkan->profiler_enabled &&
        properties().time_stamp_compute_graphics) {
        _profiler = std::make_unique<Profiler>(this);
    }
}

void Context::set_debug_name_internal(const std::string &name,
                                      uint64_t object_handle,
                                      VkObjectType object_type) {
    if (!s_vulkan->validation_enabled()) {
        return;
    }
    VkDebugUtilsObjectNameInfoEXT info{
        VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT};
    info.objectType = object_type;
    info.objectHandle = object_handle;
    info.pObjectName = name.c_str();
    instance()->SetDebugUtilsObjectNameEXT(_device->device(), &info);
}

std::shared_ptr<IPanoramaSky> Context::create_panorama_sky() {
    return std::make_shared<PanoramaSky>(this);
}

std::shared_ptr<IPhysicalSky> Context::create_physical_sky() {
    return std::make_shared<PhysicalSky>(this);
}

ImageBasedLighting *Context::ibl() const {
    return _ibl.get();
}

Handle<Texture> Context::default_texture_vk(DefaultTexture tex) {
    return upcast(default_texture(tex).get());
}

RendererContextData *Context::renderer_data() const {
    return _renderer_data.get();
}

void Context::begin_debug_label(CommandBuffer *cmd,
                                const std::string &name,
                                uint32_t color) const {
    if (!s_vulkan->validation_enabled()) {
        return;
    }

    VkDebugUtilsLabelEXT label{VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT};
    label.pLabelName = name.c_str();
    for (int i = 0; i < 4; i++) {
        auto v = static_cast<float>((color >> (8 * i)) & 0xFF) / 255.0f;
        label.color[i] = v;
    }

    instance()->CmdBeginDebugUtilsLabelEXT(cmd->command_buffer(), &label);
}

void Context::end_debug_label(CommandBuffer *cmd) const {
    if (!s_vulkan->validation_enabled()) {
        return;
    }
    instance()->CmdEndDebugUtilsLabelEXT(cmd->command_buffer());
}

std::shared_ptr<Material> Context::default_material_vk() {
    return _material_factory->default_material();
}
} // namespace ars::render::vk
