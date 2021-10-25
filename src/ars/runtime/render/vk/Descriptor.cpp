#include "Descriptor.h"
#include "Pipeline.h"
#include "Texture.h"
#include <ars/runtime/core/Log.h>
#include <cassert>

namespace ars::render::vk {

namespace {
uint32_t vk_descriptor_type_index(VkDescriptorType type) {
    switch (type) {
    case VK_DESCRIPTOR_TYPE_SAMPLER:
        return 0;
    case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
        return 1;
    case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
        return 2;
    case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
        return 3;
    case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
        return 4;
    case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
        return 5;
    case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
        return 6;
    case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
        return 7;
    case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
        return 8;
    case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
        return 9;
    case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
        return 10;
    case VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT:
        return 11;
    case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR:
        return 12;
    default:
        break;
    }
    ARS_LOG_CRITICAL("invalid descriptor type");
    return 0;
}
} // namespace

DescriptorPool::DescriptorPool(Device *device,
                               const std::vector<VkDescriptorPoolSize> &sizes,
                               uint32_t max_sets)
    : _device(device), _max_sets(max_sets) {
    for (const auto &size : sizes) {
        _sizes[vk_descriptor_type_index(size.type)] += size.descriptorCount;
    }

    VkDescriptorPoolCreateInfo info{
        VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
    info.maxSets = max_sets;
    info.poolSizeCount = static_cast<uint32_t>(sizes.size());
    info.pPoolSizes = sizes.data();

    if (_device->Create(&info, &_pool) != VK_SUCCESS) {
        ARS_LOG_CRITICAL("Failed to create descriptor pool");
    }

    reset_available_counter();
}

DescriptorPool::~DescriptorPool() {
    if (_pool != VK_NULL_HANDLE) {
        _device->Destroy(_pool);
    }
}

void DescriptorPool::reset() {
    if (_device->ResetDescriptorPool(_pool, 0) != VK_SUCCESS) {
        ARS_LOG_CRITICAL("Failed to reset descriptor pool");
    }

    reset_available_counter();
}

void DescriptorPool::alloc(VkDescriptorSetLayout layout,
                           const DescriptorSetInfo &info,
                           uint32_t count,
                           VkDescriptorSet *result) {
    if (count == 0) {
        return;
    }

    if (count > _available_sets) {
        ARS_LOG_CRITICAL("Available descriptor sets not enough");
    }

    _available_sets -= count;

    for (const auto &bind : info.bindings) {
        if (!bind.has_value()) {
            continue;
        }

        auto &available_desc_count =
            _available_sizes[vk_descriptor_type_index(bind->descriptorType)];
        if (available_desc_count < bind->descriptorCount) {
            ARS_LOG_CRITICAL("available descriptor binding not enough");
        }

        available_desc_count -= bind->descriptorCount;
    }

    std::vector<VkDescriptorSetLayout> layouts(count, layout);
    VkDescriptorSetAllocateInfo alloc_info{
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
    alloc_info.descriptorPool = _pool;
    alloc_info.descriptorSetCount = count;
    alloc_info.pSetLayouts = layouts.data();

    if (_device->Allocate(&alloc_info, result) != VK_SUCCESS) {
        ARS_LOG_CRITICAL("Failed to alloc descriptor set");
    }
}

uint32_t DescriptorPool::available_count(const DescriptorSetInfo &info) const {
    if (_available_sets == 0) {
        return 0;
    }

    uint32_t result = _available_sets;
    std::array<uint32_t, MAX_DESC_TYPE_COUNT> request{};

    for (const auto &bind : info.bindings) {
        if (!bind.has_value()) {
            continue;
        }

        request[vk_descriptor_type_index(bind->descriptorType)] +=
            bind->descriptorCount;
    }

    for (uint32_t i = 0; i < MAX_DESC_TYPE_COUNT; i++) {
        if (request[i] == 0) {
            continue;
        }

        auto available = _available_sizes[i] / request[i];
        if (available < result) {
            result = available;
        }
    }

    return result;
}

void DescriptorPool::reset_available_counter() {
    _available_sets = _max_sets;
    _available_sizes = _sizes;
}

VkDescriptorPool DescriptorPool::pool() const {
    return _pool;
}

DescriptorArena::DescriptorArena(Device *device,
                                 std::vector<VkDescriptorPoolSize> pool_sizes,
                                 uint32_t pool_max_set)
    : _device(device), _pool_sizes(std::move(pool_sizes)),
      _pool_max_set(pool_max_set) {
    alloc_new_pool();
}

void DescriptorArena::reset() {
    _current_pool_index = 0;
    for (auto &pool : _pools) {
        pool->reset();
    }
}

void DescriptorArena::alloc(VkDescriptorSetLayout layout,
                            const DescriptorSetInfo &info,
                            uint32_t count,
                            VkDescriptorSet *result) {
    if (count == 0) {
        return;
    }

    assert(!_pools.empty());
    assert(_pools.size() > _current_pool_index);

    auto available = _pools[_current_pool_index]->available_count(info);

    if (available > 0) {
        auto alloc_count = std::min(available, count);
        _pools[_current_pool_index]->alloc(layout, info, alloc_count, result);
        count -= alloc_count;
        result += alloc_count;
    }

    if (count != 0) {
        next_pool();
        alloc(layout, info, count, result);
    }
}

VkDescriptorSet DescriptorArena::alloc(VkDescriptorSetLayout layout,
                                       const DescriptorSetInfo &info) {
    VkDescriptorSet result;
    alloc(layout, info, 1, &result);
    return result;
}

void DescriptorArena::next_pool() {
    _current_pool_index += 1;
    if (_current_pool_index >= _pools.size()) {
        alloc_new_pool();
    }
}

void DescriptorArena::alloc_new_pool() {
    _pools.emplace_back(
        std::make_unique<DescriptorPool>(_device, _pool_sizes, _pool_max_set));
}

void fill_desc_combined_image_sampler(VkWriteDescriptorSet *write,
                                      VkDescriptorImageInfo *image_info,
                                      VkDescriptorSet dst_set,
                                      uint32_t binding,
                                      Texture *texture) {
    write->sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write->descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    write->dstSet = dst_set;
    write->dstBinding = binding;
    write->dstArrayElement = 0;
    write->descriptorCount = 1;
    write->pBufferInfo = nullptr;
    write->pImageInfo = image_info;
    write->pTexelBufferView = nullptr;

    image_info->imageLayout = texture->layout();
    image_info->imageView = texture->image_view();
    image_info->sampler = texture->sampler();
}

void fill_desc_uniform_buffer(VkWriteDescriptorSet *write,
                              VkDescriptorBufferInfo *buffer_info,
                              VkDescriptorSet dst_set,
                              uint32_t binding,
                              VkBuffer buffer,
                              VkDeviceSize offset,
                              VkDeviceSize range) {
    write->sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write->descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    write->dstSet = dst_set;
    write->dstBinding = binding;
    write->dstArrayElement = 0;
    write->descriptorCount = 1;
    write->pBufferInfo = buffer_info;
    write->pImageInfo = nullptr;
    write->pTexelBufferView = nullptr;

    buffer_info->buffer = buffer;
    buffer_info->offset = offset;
    buffer_info->range = range;
}
} // namespace ars::render::vk
