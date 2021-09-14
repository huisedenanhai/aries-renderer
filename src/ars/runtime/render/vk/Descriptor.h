#pragma once

#include "Vulkan.h"
#include <array>
#include <vector>

namespace ars::render::vk {
class Context;
struct DescriptorSetInfo;
class Texture;

class DescriptorPool {
  public:
    DescriptorPool(Device *device,
                   const std::vector<VkDescriptorPoolSize> &sizes,
                   uint32_t max_sets);

    ARS_NO_COPY_MOVE(DescriptorPool);

    ~DescriptorPool();

    void reset();

    [[nodiscard]] VkDescriptorPool pool() const;

    void alloc(VkDescriptorSetLayout layout,
               const DescriptorSetInfo &info,
               uint32_t count,
               VkDescriptorSet *result);

    [[nodiscard]] uint32_t available_count(const DescriptorSetInfo &info) const;

  private:
    Device *_device = nullptr;
    VkDescriptorPool _pool = VK_NULL_HANDLE;

    static constexpr uint32_t MAX_DESC_TYPE_COUNT = 16;
    std::array<uint32_t, MAX_DESC_TYPE_COUNT> _sizes{};
    std::array<uint32_t, MAX_DESC_TYPE_COUNT> _available_sizes{};

    uint32_t _max_sets = 0;
    uint32_t _available_sets = 0;

    void reset_available_counter();
};

class DescriptorArena {
  public:
    DescriptorArena(Device *device,
                    std::vector<VkDescriptorPoolSize> pool_sizes,
                    uint32_t pool_max_set);

    ARS_NO_COPY_MOVE(DescriptorArena);

    ~DescriptorArena() = default;

    void reset();

    void alloc(VkDescriptorSetLayout layout,
               const DescriptorSetInfo &info,
               uint32_t count,
               VkDescriptorSet *result);

    VkDescriptorSet alloc(VkDescriptorSetLayout layout,
                          const DescriptorSetInfo &info);

  private:
    Device *_device = nullptr;
    std::vector<std::unique_ptr<DescriptorPool>> _pools{};
    uint32_t _current_pool_index = 0;

    std::vector<VkDescriptorPoolSize> _pool_sizes;
    uint32_t _pool_max_set;

    void next_pool();
    void alloc_new_pool();
};

void fill_combined_image_sampler(VkWriteDescriptorSet *write,
                                 VkDescriptorImageInfo *image_info,
                                 VkDescriptorSet dst_set,
                                 uint32_t binding,
                                 Texture *texture);
} // namespace ars::render::vk
