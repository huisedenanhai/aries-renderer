#pragma once

#include "../Buffer.h"
#include "../Vulkan.h"

namespace ars::render::vk {
class Context;
class Mesh;
class Scene;
class View;
struct RenderGraph;

class AccelerationStructure {
  public:
    AccelerationStructure(Context *context,
                          VkAccelerationStructureTypeKHR type,
                          VkDeviceSize buffer_size);
    ~AccelerationStructure();

    [[nodiscard]] VkAccelerationStructureKHR acceleration_structure() const;
    [[nodiscard]] VkDeviceAddress device_address() const;

    static Handle<AccelerationStructure> create(Mesh *mesh);
    static Handle<AccelerationStructure> create(Scene *scene);

  private:
    static Handle<AccelerationStructure>
    create(Context *context,
           VkAccelerationStructureTypeKHR type,
           const VkAccelerationStructureGeometryKHR &geometry,
           uint32_t primitive_count);

    Context *_context = nullptr;
    VkAccelerationStructureKHR _acceleration_structure = VK_NULL_HANDLE;
    Handle<Buffer> _buffer{};
};

class RayTracing {
  public:
    explicit RayTracing(View *view);
    void render(RenderGraph &rg);
    static bool supported(Context *context);

  private:
    View *_view = nullptr;
};

} // namespace ars::render::vk