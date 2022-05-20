#pragma once

#include "../IContext.h"
#include "Buffer.h"
#include "Descriptor.h"
#include "RenderPass.h"
#include "Scene.h"
#include "Texture.h"
#include "Vulkan.h"

#include <ars/runtime/core/misc/Defer.h>

#include <set>
#include <vector>

struct GLFWwindow;

namespace ars::render::vk {
class Lut;
class DescriptorArena;
class Swapchain;
class Context;
class MaterialFactory;
class Material;
class Profiler;
class AccelerationStructure;
class ImageBasedLighting;
class RendererContextData;

void init_vulkan_backend(const ApplicationInfo &app_info);
void destroy_vulkan_backend();

// Owned by the context
class Queue {
  public:
    Queue(Context *context, uint32_t family_index);

    ARS_NO_COPY_MOVE(Queue);

    ~Queue();

    [[nodiscard]] uint32_t family_index() const;
    [[nodiscard]] VkQueue queue() const;

    void submit(CommandBuffer *command_buffer,
                uint32_t wait_semaphore_count = 0,
                VkSemaphore *wait_semaphores = nullptr);

    // record a command buffer and submit it immediately
    template <typename Func>
    auto submit_once(Func &&func,
                     uint32_t wait_semaphore_count = 0,
                     VkSemaphore *wait_semaphores = nullptr);

    // The signaled semaphore for the last submitted command.
    // If the queue is flushed, this method returns VK_NULL_HANDLE
    [[nodiscard]] VkSemaphore get_semaphore() const;

    // wait the queue idle
    void flush();

    [[nodiscard]] VkQueueFamilyProperties family_properties() const;

  private:
    // A queue for synchronizing each command buffer submissions
    std::vector<VkSemaphore> _semaphores;
    Context *_context = nullptr;
    uint32_t _family_index = 0;
    VkQueue _queue = VK_NULL_HANDLE;
    VkQueueFamilyProperties _family_properties{};
};

struct ExtensionRequirement {
  public:
    void add_extension(const std::string &name, bool required = true);

    // Those const char* will be invalid when ExtensionRequirement struct is
    // released or be modified.
    [[nodiscard]] std::vector<const char *> extensions() const;

    [[nodiscard]] std::vector<const char *> filter(
        const std::vector<VkExtensionProperties> &available_extensions) const;

    bool check(const std::vector<VkExtensionProperties> &available_extensions,
               std::string *error_str = nullptr) const;

  private:
    struct ExtensionInfo {
        bool required = false;
    };

    std::map<std::string, ExtensionInfo> _extensions{};
};

struct ContextInfo {
    VkPhysicalDeviceFeatures features{};
    VkPhysicalDeviceAccelerationStructureFeaturesKHR
        acceleration_structure_features{
            VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR};
    VkPhysicalDeviceRayTracingPipelineFeaturesKHR ray_tracing_pipeline_features{
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR};
    VkPhysicalDeviceBufferDeviceAddressFeaturesEXT device_address_features{
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_ADDRESS_FEATURES_EXT};

    VkPhysicalDeviceProperties properties{};
    VkPhysicalDeviceRayTracingPipelinePropertiesKHR
        ray_tracing_pipeline_properties{
            VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR};

    std::vector<std::string> enabled_extensions{};

    [[nodiscard]] bool support_bindless() const {
        return false;
    }

    void init(Instance *instance,
              VkPhysicalDevice physical_device,
              const ExtensionRequirement &extension_requirement);

    [[nodiscard]] std::string dump() const;
};

class Context : public IContext {
  public:
    // if window is nullptr, presentation will not be supported
    Context(const WindowInfo *window, std::unique_ptr<Swapchain> &swapchain);

    ARS_NO_COPY_MOVE(Context);

    ~Context() override;

    std::unique_ptr<IWindow> create_window(const WindowInfo &info) override;
    std::unique_ptr<IScene> create_scene() override;
    std::shared_ptr<ITexture>
    create_texture_impl(const TextureInfo &info) override;
    std::shared_ptr<IMesh> create_mesh(const MeshInfo &info) override;
    std::shared_ptr<ISkin> create_skin(const SkinInfo &info) override;
    std::shared_ptr<IMaterial>
    create_material(const MaterialInfo &info) override;
    std::shared_ptr<IPanoramaSky> create_panorama_sky() override;
    std::shared_ptr<IPhysicalSky> create_physical_sky() override;
    [[nodiscard]] Instance *instance() const;
    [[nodiscard]] Device *device() const;
    [[nodiscard]] VulkanMemoryAllocator *vma() const;
    [[nodiscard]] VkPipelineCache pipeline_cache() const;

    // The primary queue.
    //
    // This queue supports all operations: graphics, compute, transfer,
    // and presentation (if presentation is required).
    //
    // The spec says: if there are any queue supports graphics operations, there
    // must be a queue family supports both graphics and compute operations, and
    // transfer operations are implicitly supported if either graphics or
    // compute operations are supported.
    // (https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VkQueueFlagBits.html)
    //
    // It will be rare for us to find no such queue.
    [[nodiscard]] Queue *queue() const;

    [[nodiscard]] DescriptorArena *descriptor_arena() const;

    [[nodiscard]] const ContextInfo &info() const;

    bool begin_frame() override;
    void end_frame() override;

    std::shared_ptr<ITexture> default_texture(DefaultTexture tex) override;
    Handle<Texture> default_texture_vk(DefaultTexture tex);

    std::shared_ptr<Material> default_material_vk();

    std::shared_ptr<ITexture> create_single_color_texture_2d(glm::vec4 color);
    std::shared_ptr<ITexture> create_single_color_cube_map(glm::vec4 color);

    Handle<Texture> create_texture(const TextureCreateInfo &info);
    Handle<CommandBuffer> create_command_buffer(VkCommandBufferLevel level);
    Handle<Buffer> create_buffer(VkDeviceSize size,
                                 VkBufferUsageFlags buffer_usage,
                                 VmaMemoryUsage memory_usage);
    Handle<AccelerationStructure>
    create_acceleration_structure(VkAccelerationStructureTypeKHR type,
                                  VkDeviceSize buffer_size);

    Framebuffer *
    create_tmp_framebuffer(RenderPass *render_pass,
                           std::vector<Handle<Texture>> attachments);

    std::unique_ptr<Swapchain> create_swapchain(GLFWwindow *window,
                                                bool owns_window);

    // The context need to notify swapchains on frame ends so the swapchain can
    // do some necessary cleanup for cached input events
    // These two method are called by constructor and destructor of Swapchain
    void register_swapchain(Swapchain *swapchain);
    void unregister_swapchain(Swapchain *swapchain);

    [[nodiscard]] Lut *lut() const;
    [[nodiscard]] ImageBasedLighting *ibl() const;
    [[nodiscard]] Profiler *profiler() const;
    [[nodiscard]] RendererContextData *renderer_data() const;

    void set_debug_name_internal(const std::string &name,
                                 uint64_t object_handle,
                                 VkObjectType object_type);

    template <typename T>
    void set_debug_name(const std::string &name, T object_handle) {
        set_debug_name_internal(name,
                                reinterpret_cast<uint64_t>(object_handle),
                                VulkanObjectType<T>::value);
    }

    void begin_debug_label(CommandBuffer *cmd,
                           const std::string &name,
                           uint32_t color) const;

    void end_debug_label(CommandBuffer *cmd) const;

  private:
    // This method init device if not
    std::tuple<GLFWwindow *, VkSurfaceKHR>
    create_window_and_surface(const WindowInfo *info);
    // This method init device if not
    VkSurfaceKHR create_surface(GLFWwindow *window);

    void init_device_and_queues(Instance *instance,
                                bool enable_validation,
                                VkSurfaceKHR surface);
    void init_command_pool();
    void init_pipeline_cache();
    void init_descriptor_arena();
    void init_default_textures();
    void init_profiler();

    // Clear unused resources
    void gc();

    template <typename T, typename... Args>
    static Handle<T> create_handle(std::vector<std::shared_ptr<T>> &pool,
                                   Args &&...args);

    ContextInfo _info{};
    std::unique_ptr<Device> _device{};
    std::unique_ptr<Queue> _queue{};

    std::unique_ptr<VulkanMemoryAllocator> _vma{};

    std::unique_ptr<DescriptorArena> _descriptor_arena{};

    VkCommandPool _command_pool = VK_NULL_HANDLE;

    VkPipelineCache _pipeline_cache = VK_NULL_HANDLE;

    // cached resources
    std::vector<std::shared_ptr<Texture>> _textures{};
    std::vector<std::shared_ptr<CommandBuffer>> _command_buffers{};
    std::vector<std::shared_ptr<Buffer>> _buffers{};
    std::vector<std::shared_ptr<AccelerationStructure>>
        _acceleration_structures{};

    std::vector<std::unique_ptr<Framebuffer>> _tmp_framebuffers{};

    std::set<Swapchain *> _registered_swapchains{};

    std::unique_ptr<MaterialFactory> _material_factory{};

    std::array<std::shared_ptr<ITexture>,
               static_cast<size_t>(DefaultTexture::Count)>
        _default_textures{};

    std::unique_ptr<Lut> _lut{};
    std::unique_ptr<ImageBasedLighting> _ibl{};
    std::unique_ptr<Profiler> _profiler{};
    std::unique_ptr<RendererContextData> _renderer_data{};
};

template <typename Func>
auto Queue::submit_once(Func &&func,
                        uint32_t wait_semaphore_count,
                        VkSemaphore *wait_semaphores) {
    auto cmd = _context->create_command_buffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY);
    cmd->begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

    ARS_DEFER([&]() {
        cmd->end();
        submit(cmd.get(), wait_semaphore_count, wait_semaphores);
    });

    if constexpr (std::is_void_v<
                      std::invoke_result_t<Func, decltype(cmd.get())>>) {
        func(cmd.get());
    } else {
        return func(cmd.get());
    }
}

} // namespace ars::render::vk
