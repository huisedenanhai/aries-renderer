#ifndef VOLK_HPP_
#define VOLK_HPP_

#include "volk.h"

#ifndef VOLK_NAMESPACE
#define VOLK_NAMESPACE volk
#endif

namespace VOLK_NAMESPACE {
class Device {
protected:
  VkDevice _device;
  const VkAllocationCallbacks *_allocator;
  VolkDeviceTable _table;

public:
#ifndef VOLK_NO_INSTANCE_PROTOTYPES
  Device(VkDevice device, const VkAllocationCallbacks *allocator = nullptr)
      : _device(device), _allocator(allocator) {
    volkLoadDeviceTable(&_table, device);
  }
#endif
  Device(VkDevice device,
         PFN_vkGetDeviceProcAddr handle,
         const VkAllocationCallbacks *allocator = nullptr)
      : _device(device), _allocator(allocator) {
    volkLoadDeviceTableCustom(&_table, device, handle);
  }

  inline VkDevice device() const {
    return _device;
  }

  inline const VolkDeviceTable &table() const {
    return _table;
  }

  inline const VkAllocationCallbacks *allocator() const {
    return _allocator;
  }

  /* VOLK_GENERATE_DEVICE_METHOD_HPP */
#if defined(VK_VERSION_1_0)
inline VkResult Allocate(const VkCommandBufferAllocateInfo* pAllocateInfo, VkCommandBuffer* pCommandBuffers) const {
	return _table.vkAllocateCommandBuffers(_device, pAllocateInfo, pCommandBuffers);
}
inline VkResult AllocateCommandBuffers(const VkCommandBufferAllocateInfo* pAllocateInfo, VkCommandBuffer* pCommandBuffers) const {
	return _table.vkAllocateCommandBuffers(_device, pAllocateInfo, pCommandBuffers);
}
inline VkResult Allocate(const VkDescriptorSetAllocateInfo* pAllocateInfo, VkDescriptorSet* pDescriptorSets) const {
	return _table.vkAllocateDescriptorSets(_device, pAllocateInfo, pDescriptorSets);
}
inline VkResult AllocateDescriptorSets(const VkDescriptorSetAllocateInfo* pAllocateInfo, VkDescriptorSet* pDescriptorSets) const {
	return _table.vkAllocateDescriptorSets(_device, pAllocateInfo, pDescriptorSets);
}
inline VkResult Allocate(const VkMemoryAllocateInfo* pAllocateInfo, const VkAllocationCallbacks* pAllocator, VkDeviceMemory* pMemory) const {
	return _table.vkAllocateMemory(_device, pAllocateInfo, pAllocator, pMemory);
}
inline VkResult AllocateMemory(const VkMemoryAllocateInfo* pAllocateInfo, const VkAllocationCallbacks* pAllocator, VkDeviceMemory* pMemory) const {
	return _table.vkAllocateMemory(_device, pAllocateInfo, pAllocator, pMemory);
}
inline VkResult Allocate(const VkMemoryAllocateInfo* pAllocateInfo, VkDeviceMemory* pMemory) const {
	return _table.vkAllocateMemory(_device, pAllocateInfo, _allocator, pMemory);
}
inline VkResult AllocateMemory(const VkMemoryAllocateInfo* pAllocateInfo, VkDeviceMemory* pMemory) const {
	return _table.vkAllocateMemory(_device, pAllocateInfo, _allocator, pMemory);
}
inline VkResult BeginCommandBuffer(VkCommandBuffer commandBuffer, const VkCommandBufferBeginInfo* pBeginInfo) const {
	return _table.vkBeginCommandBuffer(commandBuffer, pBeginInfo);
}
inline VkResult BindBufferMemory(VkBuffer buffer, VkDeviceMemory memory, VkDeviceSize memoryOffset) const {
	return _table.vkBindBufferMemory(_device, buffer, memory, memoryOffset);
}
inline VkResult BindImageMemory(VkImage image, VkDeviceMemory memory, VkDeviceSize memoryOffset) const {
	return _table.vkBindImageMemory(_device, image, memory, memoryOffset);
}
inline void CmdBeginQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query, VkQueryControlFlags flags) const {
	_table.vkCmdBeginQuery(commandBuffer, queryPool, query, flags);
}
inline void CmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin, VkSubpassContents contents) const {
	_table.vkCmdBeginRenderPass(commandBuffer, pRenderPassBegin, contents);
}
inline void CmdBindDescriptorSets(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout, uint32_t firstSet, uint32_t descriptorSetCount, const VkDescriptorSet* pDescriptorSets, uint32_t dynamicOffsetCount, const uint32_t* pDynamicOffsets) const {
	_table.vkCmdBindDescriptorSets(commandBuffer, pipelineBindPoint, layout, firstSet, descriptorSetCount, pDescriptorSets, dynamicOffsetCount, pDynamicOffsets);
}
inline void CmdBindIndexBuffer(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkIndexType indexType) const {
	_table.vkCmdBindIndexBuffer(commandBuffer, buffer, offset, indexType);
}
inline void CmdBindPipeline(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipeline pipeline) const {
	_table.vkCmdBindPipeline(commandBuffer, pipelineBindPoint, pipeline);
}
inline void CmdBindVertexBuffers(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount, const VkBuffer* pBuffers, const VkDeviceSize* pOffsets) const {
	_table.vkCmdBindVertexBuffers(commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets);
}
inline void CmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageBlit* pRegions, VkFilter filter) const {
	_table.vkCmdBlitImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions, filter);
}
inline void CmdClearAttachments(VkCommandBuffer commandBuffer, uint32_t attachmentCount, const VkClearAttachment* pAttachments, uint32_t rectCount, const VkClearRect* pRects) const {
	_table.vkCmdClearAttachments(commandBuffer, attachmentCount, pAttachments, rectCount, pRects);
}
inline void CmdClearColorImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout, const VkClearColorValue* pColor, uint32_t rangeCount, const VkImageSubresourceRange* pRanges) const {
	_table.vkCmdClearColorImage(commandBuffer, image, imageLayout, pColor, rangeCount, pRanges);
}
inline void CmdClearDepthStencilImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout, const VkClearDepthStencilValue* pDepthStencil, uint32_t rangeCount, const VkImageSubresourceRange* pRanges) const {
	_table.vkCmdClearDepthStencilImage(commandBuffer, image, imageLayout, pDepthStencil, rangeCount, pRanges);
}
inline void CmdCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer, uint32_t regionCount, const VkBufferCopy* pRegions) const {
	_table.vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, regionCount, pRegions);
}
inline void CmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkBufferImageCopy* pRegions) const {
	_table.vkCmdCopyBufferToImage(commandBuffer, srcBuffer, dstImage, dstImageLayout, regionCount, pRegions);
}
inline void CmdCopyImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageCopy* pRegions) const {
	_table.vkCmdCopyImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions);
}
inline void CmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkBuffer dstBuffer, uint32_t regionCount, const VkBufferImageCopy* pRegions) const {
	_table.vkCmdCopyImageToBuffer(commandBuffer, srcImage, srcImageLayout, dstBuffer, regionCount, pRegions);
}
inline void CmdCopyQueryPoolResults(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize stride, VkQueryResultFlags flags) const {
	_table.vkCmdCopyQueryPoolResults(commandBuffer, queryPool, firstQuery, queryCount, dstBuffer, dstOffset, stride, flags);
}
inline void CmdDispatch(VkCommandBuffer commandBuffer, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) const {
	_table.vkCmdDispatch(commandBuffer, groupCountX, groupCountY, groupCountZ);
}
inline void CmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset) const {
	_table.vkCmdDispatchIndirect(commandBuffer, buffer, offset);
}
inline void CmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) const {
	_table.vkCmdDraw(commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
}
inline void CmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance) const {
	_table.vkCmdDrawIndexed(commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}
inline void CmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount, uint32_t stride) const {
	_table.vkCmdDrawIndexedIndirect(commandBuffer, buffer, offset, drawCount, stride);
}
inline void CmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount, uint32_t stride) const {
	_table.vkCmdDrawIndirect(commandBuffer, buffer, offset, drawCount, stride);
}
inline void CmdEndQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query) const {
	_table.vkCmdEndQuery(commandBuffer, queryPool, query);
}
inline void CmdEndRenderPass(VkCommandBuffer commandBuffer) const {
	_table.vkCmdEndRenderPass(commandBuffer);
}
inline void CmdExecuteCommands(VkCommandBuffer commandBuffer, uint32_t commandBufferCount, const VkCommandBuffer* pCommandBuffers) const {
	_table.vkCmdExecuteCommands(commandBuffer, commandBufferCount, pCommandBuffers);
}
inline void CmdFillBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize size, uint32_t data) const {
	_table.vkCmdFillBuffer(commandBuffer, dstBuffer, dstOffset, size, data);
}
inline void CmdNextSubpass(VkCommandBuffer commandBuffer, VkSubpassContents contents) const {
	_table.vkCmdNextSubpass(commandBuffer, contents);
}
inline void CmdPipelineBarrier(VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags, uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers, uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier* pImageMemoryBarriers) const {
	_table.vkCmdPipelineBarrier(commandBuffer, srcStageMask, dstStageMask, dependencyFlags, memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
}
inline void CmdPushConstants(VkCommandBuffer commandBuffer, VkPipelineLayout layout, VkShaderStageFlags stageFlags, uint32_t offset, uint32_t size, const void* pValues) const {
	_table.vkCmdPushConstants(commandBuffer, layout, stageFlags, offset, size, pValues);
}
inline void CmdResetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask) const {
	_table.vkCmdResetEvent(commandBuffer, event, stageMask);
}
inline void CmdResetQueryPool(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount) const {
	_table.vkCmdResetQueryPool(commandBuffer, queryPool, firstQuery, queryCount);
}
inline void CmdResolveImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageResolve* pRegions) const {
	_table.vkCmdResolveImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions);
}
inline void CmdSetBlendConstants(VkCommandBuffer commandBuffer, const float blendConstants[4]) const {
	_table.vkCmdSetBlendConstants(commandBuffer, blendConstants);
}
inline void CmdSetDepthBias(VkCommandBuffer commandBuffer, float depthBiasConstantFactor, float depthBiasClamp, float depthBiasSlopeFactor) const {
	_table.vkCmdSetDepthBias(commandBuffer, depthBiasConstantFactor, depthBiasClamp, depthBiasSlopeFactor);
}
inline void CmdSetDepthBounds(VkCommandBuffer commandBuffer, float minDepthBounds, float maxDepthBounds) const {
	_table.vkCmdSetDepthBounds(commandBuffer, minDepthBounds, maxDepthBounds);
}
inline void CmdSetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask) const {
	_table.vkCmdSetEvent(commandBuffer, event, stageMask);
}
inline void CmdSetLineWidth(VkCommandBuffer commandBuffer, float lineWidth) const {
	_table.vkCmdSetLineWidth(commandBuffer, lineWidth);
}
inline void CmdSetScissor(VkCommandBuffer commandBuffer, uint32_t firstScissor, uint32_t scissorCount, const VkRect2D* pScissors) const {
	_table.vkCmdSetScissor(commandBuffer, firstScissor, scissorCount, pScissors);
}
inline void CmdSetStencilCompareMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t compareMask) const {
	_table.vkCmdSetStencilCompareMask(commandBuffer, faceMask, compareMask);
}
inline void CmdSetStencilReference(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t reference) const {
	_table.vkCmdSetStencilReference(commandBuffer, faceMask, reference);
}
inline void CmdSetStencilWriteMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t writeMask) const {
	_table.vkCmdSetStencilWriteMask(commandBuffer, faceMask, writeMask);
}
inline void CmdSetViewport(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount, const VkViewport* pViewports) const {
	_table.vkCmdSetViewport(commandBuffer, firstViewport, viewportCount, pViewports);
}
inline void CmdUpdateBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize dataSize, const void* pData) const {
	_table.vkCmdUpdateBuffer(commandBuffer, dstBuffer, dstOffset, dataSize, pData);
}
inline void CmdWaitEvents(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers, uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier* pImageMemoryBarriers) const {
	_table.vkCmdWaitEvents(commandBuffer, eventCount, pEvents, srcStageMask, dstStageMask, memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
}
inline void CmdWriteTimestamp(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage, VkQueryPool queryPool, uint32_t query) const {
	_table.vkCmdWriteTimestamp(commandBuffer, pipelineStage, queryPool, query);
}
inline VkResult Create(const VkBufferCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkBuffer* pBuffer) const {
	return _table.vkCreateBuffer(_device, pCreateInfo, pAllocator, pBuffer);
}
inline VkResult CreateBuffer(const VkBufferCreateInfo* pCreateInfo, VkBuffer* pBuffer) const {
	return _table.vkCreateBuffer(_device, pCreateInfo, _allocator, pBuffer);
}
inline VkResult Create(const VkBufferCreateInfo* pCreateInfo, VkBuffer* pBuffer) const {
	return _table.vkCreateBuffer(_device, pCreateInfo, _allocator, pBuffer);
}
inline VkResult CreateBuffer(const VkBufferCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkBuffer* pBuffer) const {
	return _table.vkCreateBuffer(_device, pCreateInfo, pAllocator, pBuffer);
}
inline VkResult CreateBufferView(const VkBufferViewCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkBufferView* pView) const {
	return _table.vkCreateBufferView(_device, pCreateInfo, pAllocator, pView);
}
inline VkResult Create(const VkBufferViewCreateInfo* pCreateInfo, VkBufferView* pView) const {
	return _table.vkCreateBufferView(_device, pCreateInfo, _allocator, pView);
}
inline VkResult Create(const VkBufferViewCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkBufferView* pView) const {
	return _table.vkCreateBufferView(_device, pCreateInfo, pAllocator, pView);
}
inline VkResult CreateBufferView(const VkBufferViewCreateInfo* pCreateInfo, VkBufferView* pView) const {
	return _table.vkCreateBufferView(_device, pCreateInfo, _allocator, pView);
}
inline VkResult Create(const VkCommandPoolCreateInfo* pCreateInfo, VkCommandPool* pCommandPool) const {
	return _table.vkCreateCommandPool(_device, pCreateInfo, _allocator, pCommandPool);
}
inline VkResult CreateCommandPool(const VkCommandPoolCreateInfo* pCreateInfo, VkCommandPool* pCommandPool) const {
	return _table.vkCreateCommandPool(_device, pCreateInfo, _allocator, pCommandPool);
}
inline VkResult Create(const VkCommandPoolCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkCommandPool* pCommandPool) const {
	return _table.vkCreateCommandPool(_device, pCreateInfo, pAllocator, pCommandPool);
}
inline VkResult CreateCommandPool(const VkCommandPoolCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkCommandPool* pCommandPool) const {
	return _table.vkCreateCommandPool(_device, pCreateInfo, pAllocator, pCommandPool);
}
inline VkResult Create(VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkComputePipelineCreateInfo* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines) const {
	return _table.vkCreateComputePipelines(_device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines);
}
inline VkResult Create(VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkComputePipelineCreateInfo* pCreateInfos, VkPipeline* pPipelines) const {
	return _table.vkCreateComputePipelines(_device, pipelineCache, createInfoCount, pCreateInfos, _allocator, pPipelines);
}
inline VkResult CreateComputePipelines(VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkComputePipelineCreateInfo* pCreateInfos, VkPipeline* pPipelines) const {
	return _table.vkCreateComputePipelines(_device, pipelineCache, createInfoCount, pCreateInfos, _allocator, pPipelines);
}
inline VkResult CreateComputePipelines(VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkComputePipelineCreateInfo* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines) const {
	return _table.vkCreateComputePipelines(_device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines);
}
inline VkResult CreateDescriptorPool(const VkDescriptorPoolCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDescriptorPool* pDescriptorPool) const {
	return _table.vkCreateDescriptorPool(_device, pCreateInfo, pAllocator, pDescriptorPool);
}
inline VkResult CreateDescriptorPool(const VkDescriptorPoolCreateInfo* pCreateInfo, VkDescriptorPool* pDescriptorPool) const {
	return _table.vkCreateDescriptorPool(_device, pCreateInfo, _allocator, pDescriptorPool);
}
inline VkResult Create(const VkDescriptorPoolCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDescriptorPool* pDescriptorPool) const {
	return _table.vkCreateDescriptorPool(_device, pCreateInfo, pAllocator, pDescriptorPool);
}
inline VkResult Create(const VkDescriptorPoolCreateInfo* pCreateInfo, VkDescriptorPool* pDescriptorPool) const {
	return _table.vkCreateDescriptorPool(_device, pCreateInfo, _allocator, pDescriptorPool);
}
inline VkResult Create(const VkDescriptorSetLayoutCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDescriptorSetLayout* pSetLayout) const {
	return _table.vkCreateDescriptorSetLayout(_device, pCreateInfo, pAllocator, pSetLayout);
}
inline VkResult CreateDescriptorSetLayout(const VkDescriptorSetLayoutCreateInfo* pCreateInfo, VkDescriptorSetLayout* pSetLayout) const {
	return _table.vkCreateDescriptorSetLayout(_device, pCreateInfo, _allocator, pSetLayout);
}
inline VkResult Create(const VkDescriptorSetLayoutCreateInfo* pCreateInfo, VkDescriptorSetLayout* pSetLayout) const {
	return _table.vkCreateDescriptorSetLayout(_device, pCreateInfo, _allocator, pSetLayout);
}
inline VkResult CreateDescriptorSetLayout(const VkDescriptorSetLayoutCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDescriptorSetLayout* pSetLayout) const {
	return _table.vkCreateDescriptorSetLayout(_device, pCreateInfo, pAllocator, pSetLayout);
}
inline VkResult Create(const VkEventCreateInfo* pCreateInfo, VkEvent* pEvent) const {
	return _table.vkCreateEvent(_device, pCreateInfo, _allocator, pEvent);
}
inline VkResult CreateEvent(const VkEventCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkEvent* pEvent) const {
	return _table.vkCreateEvent(_device, pCreateInfo, pAllocator, pEvent);
}
inline VkResult Create(const VkEventCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkEvent* pEvent) const {
	return _table.vkCreateEvent(_device, pCreateInfo, pAllocator, pEvent);
}
inline VkResult CreateEvent(const VkEventCreateInfo* pCreateInfo, VkEvent* pEvent) const {
	return _table.vkCreateEvent(_device, pCreateInfo, _allocator, pEvent);
}
inline VkResult CreateFence(const VkFenceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkFence* pFence) const {
	return _table.vkCreateFence(_device, pCreateInfo, pAllocator, pFence);
}
inline VkResult CreateFence(const VkFenceCreateInfo* pCreateInfo, VkFence* pFence) const {
	return _table.vkCreateFence(_device, pCreateInfo, _allocator, pFence);
}
inline VkResult Create(const VkFenceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkFence* pFence) const {
	return _table.vkCreateFence(_device, pCreateInfo, pAllocator, pFence);
}
inline VkResult Create(const VkFenceCreateInfo* pCreateInfo, VkFence* pFence) const {
	return _table.vkCreateFence(_device, pCreateInfo, _allocator, pFence);
}
inline VkResult Create(const VkFramebufferCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkFramebuffer* pFramebuffer) const {
	return _table.vkCreateFramebuffer(_device, pCreateInfo, pAllocator, pFramebuffer);
}
inline VkResult Create(const VkFramebufferCreateInfo* pCreateInfo, VkFramebuffer* pFramebuffer) const {
	return _table.vkCreateFramebuffer(_device, pCreateInfo, _allocator, pFramebuffer);
}
inline VkResult CreateFramebuffer(const VkFramebufferCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkFramebuffer* pFramebuffer) const {
	return _table.vkCreateFramebuffer(_device, pCreateInfo, pAllocator, pFramebuffer);
}
inline VkResult CreateFramebuffer(const VkFramebufferCreateInfo* pCreateInfo, VkFramebuffer* pFramebuffer) const {
	return _table.vkCreateFramebuffer(_device, pCreateInfo, _allocator, pFramebuffer);
}
inline VkResult Create(VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkGraphicsPipelineCreateInfo* pCreateInfos, VkPipeline* pPipelines) const {
	return _table.vkCreateGraphicsPipelines(_device, pipelineCache, createInfoCount, pCreateInfos, _allocator, pPipelines);
}
inline VkResult CreateGraphicsPipelines(VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkGraphicsPipelineCreateInfo* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines) const {
	return _table.vkCreateGraphicsPipelines(_device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines);
}
inline VkResult Create(VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkGraphicsPipelineCreateInfo* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines) const {
	return _table.vkCreateGraphicsPipelines(_device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines);
}
inline VkResult CreateGraphicsPipelines(VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkGraphicsPipelineCreateInfo* pCreateInfos, VkPipeline* pPipelines) const {
	return _table.vkCreateGraphicsPipelines(_device, pipelineCache, createInfoCount, pCreateInfos, _allocator, pPipelines);
}
inline VkResult CreateImage(const VkImageCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkImage* pImage) const {
	return _table.vkCreateImage(_device, pCreateInfo, pAllocator, pImage);
}
inline VkResult Create(const VkImageCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkImage* pImage) const {
	return _table.vkCreateImage(_device, pCreateInfo, pAllocator, pImage);
}
inline VkResult Create(const VkImageCreateInfo* pCreateInfo, VkImage* pImage) const {
	return _table.vkCreateImage(_device, pCreateInfo, _allocator, pImage);
}
inline VkResult CreateImage(const VkImageCreateInfo* pCreateInfo, VkImage* pImage) const {
	return _table.vkCreateImage(_device, pCreateInfo, _allocator, pImage);
}
inline VkResult CreateImageView(const VkImageViewCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkImageView* pView) const {
	return _table.vkCreateImageView(_device, pCreateInfo, pAllocator, pView);
}
inline VkResult Create(const VkImageViewCreateInfo* pCreateInfo, VkImageView* pView) const {
	return _table.vkCreateImageView(_device, pCreateInfo, _allocator, pView);
}
inline VkResult Create(const VkImageViewCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkImageView* pView) const {
	return _table.vkCreateImageView(_device, pCreateInfo, pAllocator, pView);
}
inline VkResult CreateImageView(const VkImageViewCreateInfo* pCreateInfo, VkImageView* pView) const {
	return _table.vkCreateImageView(_device, pCreateInfo, _allocator, pView);
}
inline VkResult CreatePipelineCache(const VkPipelineCacheCreateInfo* pCreateInfo, VkPipelineCache* pPipelineCache) const {
	return _table.vkCreatePipelineCache(_device, pCreateInfo, _allocator, pPipelineCache);
}
inline VkResult CreatePipelineCache(const VkPipelineCacheCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkPipelineCache* pPipelineCache) const {
	return _table.vkCreatePipelineCache(_device, pCreateInfo, pAllocator, pPipelineCache);
}
inline VkResult Create(const VkPipelineCacheCreateInfo* pCreateInfo, VkPipelineCache* pPipelineCache) const {
	return _table.vkCreatePipelineCache(_device, pCreateInfo, _allocator, pPipelineCache);
}
inline VkResult Create(const VkPipelineCacheCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkPipelineCache* pPipelineCache) const {
	return _table.vkCreatePipelineCache(_device, pCreateInfo, pAllocator, pPipelineCache);
}
inline VkResult Create(const VkPipelineLayoutCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkPipelineLayout* pPipelineLayout) const {
	return _table.vkCreatePipelineLayout(_device, pCreateInfo, pAllocator, pPipelineLayout);
}
inline VkResult Create(const VkPipelineLayoutCreateInfo* pCreateInfo, VkPipelineLayout* pPipelineLayout) const {
	return _table.vkCreatePipelineLayout(_device, pCreateInfo, _allocator, pPipelineLayout);
}
inline VkResult CreatePipelineLayout(const VkPipelineLayoutCreateInfo* pCreateInfo, VkPipelineLayout* pPipelineLayout) const {
	return _table.vkCreatePipelineLayout(_device, pCreateInfo, _allocator, pPipelineLayout);
}
inline VkResult CreatePipelineLayout(const VkPipelineLayoutCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkPipelineLayout* pPipelineLayout) const {
	return _table.vkCreatePipelineLayout(_device, pCreateInfo, pAllocator, pPipelineLayout);
}
inline VkResult Create(const VkQueryPoolCreateInfo* pCreateInfo, VkQueryPool* pQueryPool) const {
	return _table.vkCreateQueryPool(_device, pCreateInfo, _allocator, pQueryPool);
}
inline VkResult CreateQueryPool(const VkQueryPoolCreateInfo* pCreateInfo, VkQueryPool* pQueryPool) const {
	return _table.vkCreateQueryPool(_device, pCreateInfo, _allocator, pQueryPool);
}
inline VkResult CreateQueryPool(const VkQueryPoolCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkQueryPool* pQueryPool) const {
	return _table.vkCreateQueryPool(_device, pCreateInfo, pAllocator, pQueryPool);
}
inline VkResult Create(const VkQueryPoolCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkQueryPool* pQueryPool) const {
	return _table.vkCreateQueryPool(_device, pCreateInfo, pAllocator, pQueryPool);
}
inline VkResult CreateRenderPass(const VkRenderPassCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass) const {
	return _table.vkCreateRenderPass(_device, pCreateInfo, pAllocator, pRenderPass);
}
inline VkResult Create(const VkRenderPassCreateInfo* pCreateInfo, VkRenderPass* pRenderPass) const {
	return _table.vkCreateRenderPass(_device, pCreateInfo, _allocator, pRenderPass);
}
inline VkResult CreateRenderPass(const VkRenderPassCreateInfo* pCreateInfo, VkRenderPass* pRenderPass) const {
	return _table.vkCreateRenderPass(_device, pCreateInfo, _allocator, pRenderPass);
}
inline VkResult Create(const VkRenderPassCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass) const {
	return _table.vkCreateRenderPass(_device, pCreateInfo, pAllocator, pRenderPass);
}
inline VkResult CreateSampler(const VkSamplerCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSampler* pSampler) const {
	return _table.vkCreateSampler(_device, pCreateInfo, pAllocator, pSampler);
}
inline VkResult Create(const VkSamplerCreateInfo* pCreateInfo, VkSampler* pSampler) const {
	return _table.vkCreateSampler(_device, pCreateInfo, _allocator, pSampler);
}
inline VkResult CreateSampler(const VkSamplerCreateInfo* pCreateInfo, VkSampler* pSampler) const {
	return _table.vkCreateSampler(_device, pCreateInfo, _allocator, pSampler);
}
inline VkResult Create(const VkSamplerCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSampler* pSampler) const {
	return _table.vkCreateSampler(_device, pCreateInfo, pAllocator, pSampler);
}
inline VkResult Create(const VkSemaphoreCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSemaphore* pSemaphore) const {
	return _table.vkCreateSemaphore(_device, pCreateInfo, pAllocator, pSemaphore);
}
inline VkResult CreateSemaphore(const VkSemaphoreCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSemaphore* pSemaphore) const {
	return _table.vkCreateSemaphore(_device, pCreateInfo, pAllocator, pSemaphore);
}
inline VkResult CreateSemaphore(const VkSemaphoreCreateInfo* pCreateInfo, VkSemaphore* pSemaphore) const {
	return _table.vkCreateSemaphore(_device, pCreateInfo, _allocator, pSemaphore);
}
inline VkResult Create(const VkSemaphoreCreateInfo* pCreateInfo, VkSemaphore* pSemaphore) const {
	return _table.vkCreateSemaphore(_device, pCreateInfo, _allocator, pSemaphore);
}
inline VkResult Create(const VkShaderModuleCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkShaderModule* pShaderModule) const {
	return _table.vkCreateShaderModule(_device, pCreateInfo, pAllocator, pShaderModule);
}
inline VkResult Create(const VkShaderModuleCreateInfo* pCreateInfo, VkShaderModule* pShaderModule) const {
	return _table.vkCreateShaderModule(_device, pCreateInfo, _allocator, pShaderModule);
}
inline VkResult CreateShaderModule(const VkShaderModuleCreateInfo* pCreateInfo, VkShaderModule* pShaderModule) const {
	return _table.vkCreateShaderModule(_device, pCreateInfo, _allocator, pShaderModule);
}
inline VkResult CreateShaderModule(const VkShaderModuleCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkShaderModule* pShaderModule) const {
	return _table.vkCreateShaderModule(_device, pCreateInfo, pAllocator, pShaderModule);
}
inline void Destroy(VkBuffer buffer, const VkAllocationCallbacks* pAllocator) const {
	_table.vkDestroyBuffer(_device, buffer, pAllocator);
}
inline void Destroy(VkBuffer buffer) const {
	_table.vkDestroyBuffer(_device, buffer, _allocator);
}
inline void DestroyBuffer(VkBuffer buffer) const {
	_table.vkDestroyBuffer(_device, buffer, _allocator);
}
inline void DestroyBuffer(VkBuffer buffer, const VkAllocationCallbacks* pAllocator) const {
	_table.vkDestroyBuffer(_device, buffer, pAllocator);
}
inline void DestroyBufferView(VkBufferView bufferView) const {
	_table.vkDestroyBufferView(_device, bufferView, _allocator);
}
inline void DestroyBufferView(VkBufferView bufferView, const VkAllocationCallbacks* pAllocator) const {
	_table.vkDestroyBufferView(_device, bufferView, pAllocator);
}
inline void Destroy(VkBufferView bufferView) const {
	_table.vkDestroyBufferView(_device, bufferView, _allocator);
}
inline void Destroy(VkBufferView bufferView, const VkAllocationCallbacks* pAllocator) const {
	_table.vkDestroyBufferView(_device, bufferView, pAllocator);
}
inline void Destroy(VkCommandPool commandPool) const {
	_table.vkDestroyCommandPool(_device, commandPool, _allocator);
}
inline void DestroyCommandPool(VkCommandPool commandPool, const VkAllocationCallbacks* pAllocator) const {
	_table.vkDestroyCommandPool(_device, commandPool, pAllocator);
}
inline void DestroyCommandPool(VkCommandPool commandPool) const {
	_table.vkDestroyCommandPool(_device, commandPool, _allocator);
}
inline void Destroy(VkCommandPool commandPool, const VkAllocationCallbacks* pAllocator) const {
	_table.vkDestroyCommandPool(_device, commandPool, pAllocator);
}
inline void Destroy(VkDescriptorPool descriptorPool) const {
	_table.vkDestroyDescriptorPool(_device, descriptorPool, _allocator);
}
inline void Destroy(VkDescriptorPool descriptorPool, const VkAllocationCallbacks* pAllocator) const {
	_table.vkDestroyDescriptorPool(_device, descriptorPool, pAllocator);
}
inline void DestroyDescriptorPool(VkDescriptorPool descriptorPool, const VkAllocationCallbacks* pAllocator) const {
	_table.vkDestroyDescriptorPool(_device, descriptorPool, pAllocator);
}
inline void DestroyDescriptorPool(VkDescriptorPool descriptorPool) const {
	_table.vkDestroyDescriptorPool(_device, descriptorPool, _allocator);
}
inline void Destroy(VkDescriptorSetLayout descriptorSetLayout, const VkAllocationCallbacks* pAllocator) const {
	_table.vkDestroyDescriptorSetLayout(_device, descriptorSetLayout, pAllocator);
}
inline void DestroyDescriptorSetLayout(VkDescriptorSetLayout descriptorSetLayout) const {
	_table.vkDestroyDescriptorSetLayout(_device, descriptorSetLayout, _allocator);
}
inline void DestroyDescriptorSetLayout(VkDescriptorSetLayout descriptorSetLayout, const VkAllocationCallbacks* pAllocator) const {
	_table.vkDestroyDescriptorSetLayout(_device, descriptorSetLayout, pAllocator);
}
inline void Destroy(VkDescriptorSetLayout descriptorSetLayout) const {
	_table.vkDestroyDescriptorSetLayout(_device, descriptorSetLayout, _allocator);
}
inline void Destroy() const {
	_table.vkDestroyDevice(_device, _allocator);
}
inline void DestroyDevice(const VkAllocationCallbacks* pAllocator) const {
	_table.vkDestroyDevice(_device, pAllocator);
}
inline void DestroyDevice() const {
	_table.vkDestroyDevice(_device, _allocator);
}
inline void Destroy(const VkAllocationCallbacks* pAllocator) const {
	_table.vkDestroyDevice(_device, pAllocator);
}
inline void DestroyEvent(VkEvent event) const {
	_table.vkDestroyEvent(_device, event, _allocator);
}
inline void Destroy(VkEvent event) const {
	_table.vkDestroyEvent(_device, event, _allocator);
}
inline void DestroyEvent(VkEvent event, const VkAllocationCallbacks* pAllocator) const {
	_table.vkDestroyEvent(_device, event, pAllocator);
}
inline void Destroy(VkEvent event, const VkAllocationCallbacks* pAllocator) const {
	_table.vkDestroyEvent(_device, event, pAllocator);
}
inline void Destroy(VkFence fence) const {
	_table.vkDestroyFence(_device, fence, _allocator);
}
inline void DestroyFence(VkFence fence, const VkAllocationCallbacks* pAllocator) const {
	_table.vkDestroyFence(_device, fence, pAllocator);
}
inline void Destroy(VkFence fence, const VkAllocationCallbacks* pAllocator) const {
	_table.vkDestroyFence(_device, fence, pAllocator);
}
inline void DestroyFence(VkFence fence) const {
	_table.vkDestroyFence(_device, fence, _allocator);
}
inline void DestroyFramebuffer(VkFramebuffer framebuffer) const {
	_table.vkDestroyFramebuffer(_device, framebuffer, _allocator);
}
inline void DestroyFramebuffer(VkFramebuffer framebuffer, const VkAllocationCallbacks* pAllocator) const {
	_table.vkDestroyFramebuffer(_device, framebuffer, pAllocator);
}
inline void Destroy(VkFramebuffer framebuffer) const {
	_table.vkDestroyFramebuffer(_device, framebuffer, _allocator);
}
inline void Destroy(VkFramebuffer framebuffer, const VkAllocationCallbacks* pAllocator) const {
	_table.vkDestroyFramebuffer(_device, framebuffer, pAllocator);
}
inline void Destroy(VkImage image) const {
	_table.vkDestroyImage(_device, image, _allocator);
}
inline void DestroyImage(VkImage image, const VkAllocationCallbacks* pAllocator) const {
	_table.vkDestroyImage(_device, image, pAllocator);
}
inline void Destroy(VkImage image, const VkAllocationCallbacks* pAllocator) const {
	_table.vkDestroyImage(_device, image, pAllocator);
}
inline void DestroyImage(VkImage image) const {
	_table.vkDestroyImage(_device, image, _allocator);
}
inline void DestroyImageView(VkImageView imageView) const {
	_table.vkDestroyImageView(_device, imageView, _allocator);
}
inline void DestroyImageView(VkImageView imageView, const VkAllocationCallbacks* pAllocator) const {
	_table.vkDestroyImageView(_device, imageView, pAllocator);
}
inline void Destroy(VkImageView imageView) const {
	_table.vkDestroyImageView(_device, imageView, _allocator);
}
inline void Destroy(VkImageView imageView, const VkAllocationCallbacks* pAllocator) const {
	_table.vkDestroyImageView(_device, imageView, pAllocator);
}
inline void Destroy(VkPipeline pipeline, const VkAllocationCallbacks* pAllocator) const {
	_table.vkDestroyPipeline(_device, pipeline, pAllocator);
}
inline void DestroyPipeline(VkPipeline pipeline) const {
	_table.vkDestroyPipeline(_device, pipeline, _allocator);
}
inline void Destroy(VkPipeline pipeline) const {
	_table.vkDestroyPipeline(_device, pipeline, _allocator);
}
inline void DestroyPipeline(VkPipeline pipeline, const VkAllocationCallbacks* pAllocator) const {
	_table.vkDestroyPipeline(_device, pipeline, pAllocator);
}
inline void DestroyPipelineCache(VkPipelineCache pipelineCache, const VkAllocationCallbacks* pAllocator) const {
	_table.vkDestroyPipelineCache(_device, pipelineCache, pAllocator);
}
inline void DestroyPipelineCache(VkPipelineCache pipelineCache) const {
	_table.vkDestroyPipelineCache(_device, pipelineCache, _allocator);
}
inline void Destroy(VkPipelineCache pipelineCache, const VkAllocationCallbacks* pAllocator) const {
	_table.vkDestroyPipelineCache(_device, pipelineCache, pAllocator);
}
inline void Destroy(VkPipelineCache pipelineCache) const {
	_table.vkDestroyPipelineCache(_device, pipelineCache, _allocator);
}
inline void Destroy(VkPipelineLayout pipelineLayout) const {
	_table.vkDestroyPipelineLayout(_device, pipelineLayout, _allocator);
}
inline void DestroyPipelineLayout(VkPipelineLayout pipelineLayout, const VkAllocationCallbacks* pAllocator) const {
	_table.vkDestroyPipelineLayout(_device, pipelineLayout, pAllocator);
}
inline void Destroy(VkPipelineLayout pipelineLayout, const VkAllocationCallbacks* pAllocator) const {
	_table.vkDestroyPipelineLayout(_device, pipelineLayout, pAllocator);
}
inline void DestroyPipelineLayout(VkPipelineLayout pipelineLayout) const {
	_table.vkDestroyPipelineLayout(_device, pipelineLayout, _allocator);
}
inline void DestroyQueryPool(VkQueryPool queryPool) const {
	_table.vkDestroyQueryPool(_device, queryPool, _allocator);
}
inline void Destroy(VkQueryPool queryPool) const {
	_table.vkDestroyQueryPool(_device, queryPool, _allocator);
}
inline void Destroy(VkQueryPool queryPool, const VkAllocationCallbacks* pAllocator) const {
	_table.vkDestroyQueryPool(_device, queryPool, pAllocator);
}
inline void DestroyQueryPool(VkQueryPool queryPool, const VkAllocationCallbacks* pAllocator) const {
	_table.vkDestroyQueryPool(_device, queryPool, pAllocator);
}
inline void DestroyRenderPass(VkRenderPass renderPass) const {
	_table.vkDestroyRenderPass(_device, renderPass, _allocator);
}
inline void DestroyRenderPass(VkRenderPass renderPass, const VkAllocationCallbacks* pAllocator) const {
	_table.vkDestroyRenderPass(_device, renderPass, pAllocator);
}
inline void Destroy(VkRenderPass renderPass, const VkAllocationCallbacks* pAllocator) const {
	_table.vkDestroyRenderPass(_device, renderPass, pAllocator);
}
inline void Destroy(VkRenderPass renderPass) const {
	_table.vkDestroyRenderPass(_device, renderPass, _allocator);
}
inline void Destroy(VkSampler sampler, const VkAllocationCallbacks* pAllocator) const {
	_table.vkDestroySampler(_device, sampler, pAllocator);
}
inline void Destroy(VkSampler sampler) const {
	_table.vkDestroySampler(_device, sampler, _allocator);
}
inline void DestroySampler(VkSampler sampler) const {
	_table.vkDestroySampler(_device, sampler, _allocator);
}
inline void DestroySampler(VkSampler sampler, const VkAllocationCallbacks* pAllocator) const {
	_table.vkDestroySampler(_device, sampler, pAllocator);
}
inline void DestroySemaphore(VkSemaphore semaphore) const {
	_table.vkDestroySemaphore(_device, semaphore, _allocator);
}
inline void Destroy(VkSemaphore semaphore, const VkAllocationCallbacks* pAllocator) const {
	_table.vkDestroySemaphore(_device, semaphore, pAllocator);
}
inline void Destroy(VkSemaphore semaphore) const {
	_table.vkDestroySemaphore(_device, semaphore, _allocator);
}
inline void DestroySemaphore(VkSemaphore semaphore, const VkAllocationCallbacks* pAllocator) const {
	_table.vkDestroySemaphore(_device, semaphore, pAllocator);
}
inline void DestroyShaderModule(VkShaderModule shaderModule) const {
	_table.vkDestroyShaderModule(_device, shaderModule, _allocator);
}
inline void Destroy(VkShaderModule shaderModule, const VkAllocationCallbacks* pAllocator) const {
	_table.vkDestroyShaderModule(_device, shaderModule, pAllocator);
}
inline void Destroy(VkShaderModule shaderModule) const {
	_table.vkDestroyShaderModule(_device, shaderModule, _allocator);
}
inline void DestroyShaderModule(VkShaderModule shaderModule, const VkAllocationCallbacks* pAllocator) const {
	_table.vkDestroyShaderModule(_device, shaderModule, pAllocator);
}
inline VkResult DeviceWaitIdle() const {
	return _table.vkDeviceWaitIdle(_device);
}
inline VkResult EndCommandBuffer(VkCommandBuffer commandBuffer) const {
	return _table.vkEndCommandBuffer(commandBuffer);
}
inline VkResult FlushMappedMemoryRanges(uint32_t memoryRangeCount, const VkMappedMemoryRange* pMemoryRanges) const {
	return _table.vkFlushMappedMemoryRanges(_device, memoryRangeCount, pMemoryRanges);
}
inline void Free(VkCommandPool commandPool, uint32_t commandBufferCount, const VkCommandBuffer* pCommandBuffers) const {
	_table.vkFreeCommandBuffers(_device, commandPool, commandBufferCount, pCommandBuffers);
}
inline void FreeCommandBuffers(VkCommandPool commandPool, uint32_t commandBufferCount, const VkCommandBuffer* pCommandBuffers) const {
	_table.vkFreeCommandBuffers(_device, commandPool, commandBufferCount, pCommandBuffers);
}
inline VkResult Free(VkDescriptorPool descriptorPool, uint32_t descriptorSetCount, const VkDescriptorSet* pDescriptorSets) const {
	return _table.vkFreeDescriptorSets(_device, descriptorPool, descriptorSetCount, pDescriptorSets);
}
inline VkResult FreeDescriptorSets(VkDescriptorPool descriptorPool, uint32_t descriptorSetCount, const VkDescriptorSet* pDescriptorSets) const {
	return _table.vkFreeDescriptorSets(_device, descriptorPool, descriptorSetCount, pDescriptorSets);
}
inline void FreeMemory(VkDeviceMemory memory, const VkAllocationCallbacks* pAllocator) const {
	_table.vkFreeMemory(_device, memory, pAllocator);
}
inline void Free(VkDeviceMemory memory, const VkAllocationCallbacks* pAllocator) const {
	_table.vkFreeMemory(_device, memory, pAllocator);
}
inline void Free(VkDeviceMemory memory) const {
	_table.vkFreeMemory(_device, memory, _allocator);
}
inline void FreeMemory(VkDeviceMemory memory) const {
	_table.vkFreeMemory(_device, memory, _allocator);
}
inline void GetBufferMemoryRequirements(VkBuffer buffer, VkMemoryRequirements* pMemoryRequirements) const {
	_table.vkGetBufferMemoryRequirements(_device, buffer, pMemoryRequirements);
}
inline void GetDeviceMemoryCommitment(VkDeviceMemory memory, VkDeviceSize* pCommittedMemoryInBytes) const {
	_table.vkGetDeviceMemoryCommitment(_device, memory, pCommittedMemoryInBytes);
}
inline void GetDeviceQueue(uint32_t queueFamilyIndex, uint32_t queueIndex, VkQueue* pQueue) const {
	_table.vkGetDeviceQueue(_device, queueFamilyIndex, queueIndex, pQueue);
}
inline VkResult GetEventStatus(VkEvent event) const {
	return _table.vkGetEventStatus(_device, event);
}
inline VkResult GetFenceStatus(VkFence fence) const {
	return _table.vkGetFenceStatus(_device, fence);
}
inline void GetImageMemoryRequirements(VkImage image, VkMemoryRequirements* pMemoryRequirements) const {
	_table.vkGetImageMemoryRequirements(_device, image, pMemoryRequirements);
}
inline void GetImageSparseMemoryRequirements(VkImage image, uint32_t* pSparseMemoryRequirementCount, VkSparseImageMemoryRequirements* pSparseMemoryRequirements) const {
	_table.vkGetImageSparseMemoryRequirements(_device, image, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
}
inline void GetImageSubresourceLayout(VkImage image, const VkImageSubresource* pSubresource, VkSubresourceLayout* pLayout) const {
	_table.vkGetImageSubresourceLayout(_device, image, pSubresource, pLayout);
}
inline VkResult GetPipelineCacheData(VkPipelineCache pipelineCache, size_t* pDataSize, void* pData) const {
	return _table.vkGetPipelineCacheData(_device, pipelineCache, pDataSize, pData);
}
inline VkResult GetQueryPoolResults(VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount, size_t dataSize, void* pData, VkDeviceSize stride, VkQueryResultFlags flags) const {
	return _table.vkGetQueryPoolResults(_device, queryPool, firstQuery, queryCount, dataSize, pData, stride, flags);
}
inline void GetRenderAreaGranularity(VkRenderPass renderPass, VkExtent2D* pGranularity) const {
	_table.vkGetRenderAreaGranularity(_device, renderPass, pGranularity);
}
inline VkResult InvalidateMappedMemoryRanges(uint32_t memoryRangeCount, const VkMappedMemoryRange* pMemoryRanges) const {
	return _table.vkInvalidateMappedMemoryRanges(_device, memoryRangeCount, pMemoryRanges);
}
inline VkResult MapMemory(VkDeviceMemory memory, VkDeviceSize offset, VkDeviceSize size, VkMemoryMapFlags flags, void** ppData) const {
	return _table.vkMapMemory(_device, memory, offset, size, flags, ppData);
}
inline VkResult MergePipelineCaches(VkPipelineCache dstCache, uint32_t srcCacheCount, const VkPipelineCache* pSrcCaches) const {
	return _table.vkMergePipelineCaches(_device, dstCache, srcCacheCount, pSrcCaches);
}
inline VkResult QueueBindSparse(VkQueue queue, uint32_t bindInfoCount, const VkBindSparseInfo* pBindInfo, VkFence fence) const {
	return _table.vkQueueBindSparse(queue, bindInfoCount, pBindInfo, fence);
}
inline VkResult QueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo* pSubmits, VkFence fence) const {
	return _table.vkQueueSubmit(queue, submitCount, pSubmits, fence);
}
inline VkResult QueueWaitIdle(VkQueue queue) const {
	return _table.vkQueueWaitIdle(queue);
}
inline VkResult ResetCommandBuffer(VkCommandBuffer commandBuffer, VkCommandBufferResetFlags flags) const {
	return _table.vkResetCommandBuffer(commandBuffer, flags);
}
inline VkResult ResetCommandPool(VkCommandPool commandPool, VkCommandPoolResetFlags flags) const {
	return _table.vkResetCommandPool(_device, commandPool, flags);
}
inline VkResult ResetDescriptorPool(VkDescriptorPool descriptorPool, VkDescriptorPoolResetFlags flags) const {
	return _table.vkResetDescriptorPool(_device, descriptorPool, flags);
}
inline VkResult ResetEvent(VkEvent event) const {
	return _table.vkResetEvent(_device, event);
}
inline VkResult ResetFences(uint32_t fenceCount, const VkFence* pFences) const {
	return _table.vkResetFences(_device, fenceCount, pFences);
}
inline VkResult SetEvent(VkEvent event) const {
	return _table.vkSetEvent(_device, event);
}
inline void UnmapMemory(VkDeviceMemory memory) const {
	_table.vkUnmapMemory(_device, memory);
}
inline void UpdateDescriptorSets(uint32_t descriptorWriteCount, const VkWriteDescriptorSet* pDescriptorWrites, uint32_t descriptorCopyCount, const VkCopyDescriptorSet* pDescriptorCopies) const {
	_table.vkUpdateDescriptorSets(_device, descriptorWriteCount, pDescriptorWrites, descriptorCopyCount, pDescriptorCopies);
}
inline VkResult WaitForFences(uint32_t fenceCount, const VkFence* pFences, VkBool32 waitAll, uint64_t timeout) const {
	return _table.vkWaitForFences(_device, fenceCount, pFences, waitAll, timeout);
}
#endif /* defined(VK_VERSION_1_0) */
#if defined(VK_VERSION_1_1)
inline VkResult BindBufferMemory2(uint32_t bindInfoCount, const VkBindBufferMemoryInfo* pBindInfos) const {
	return _table.vkBindBufferMemory2(_device, bindInfoCount, pBindInfos);
}
inline VkResult BindImageMemory2(uint32_t bindInfoCount, const VkBindImageMemoryInfo* pBindInfos) const {
	return _table.vkBindImageMemory2(_device, bindInfoCount, pBindInfos);
}
inline void CmdDispatchBase(VkCommandBuffer commandBuffer, uint32_t baseGroupX, uint32_t baseGroupY, uint32_t baseGroupZ, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) const {
	_table.vkCmdDispatchBase(commandBuffer, baseGroupX, baseGroupY, baseGroupZ, groupCountX, groupCountY, groupCountZ);
}
inline void CmdSetDeviceMask(VkCommandBuffer commandBuffer, uint32_t deviceMask) const {
	_table.vkCmdSetDeviceMask(commandBuffer, deviceMask);
}
inline VkResult CreateDescriptorUpdateTemplate(const VkDescriptorUpdateTemplateCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDescriptorUpdateTemplate* pDescriptorUpdateTemplate) const {
	return _table.vkCreateDescriptorUpdateTemplate(_device, pCreateInfo, pAllocator, pDescriptorUpdateTemplate);
}
inline VkResult Create(const VkDescriptorUpdateTemplateCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDescriptorUpdateTemplate* pDescriptorUpdateTemplate) const {
	return _table.vkCreateDescriptorUpdateTemplate(_device, pCreateInfo, pAllocator, pDescriptorUpdateTemplate);
}
inline VkResult CreateDescriptorUpdateTemplate(const VkDescriptorUpdateTemplateCreateInfo* pCreateInfo, VkDescriptorUpdateTemplate* pDescriptorUpdateTemplate) const {
	return _table.vkCreateDescriptorUpdateTemplate(_device, pCreateInfo, _allocator, pDescriptorUpdateTemplate);
}
inline VkResult Create(const VkDescriptorUpdateTemplateCreateInfo* pCreateInfo, VkDescriptorUpdateTemplate* pDescriptorUpdateTemplate) const {
	return _table.vkCreateDescriptorUpdateTemplate(_device, pCreateInfo, _allocator, pDescriptorUpdateTemplate);
}
inline VkResult CreateSamplerYcbcrConversion(const VkSamplerYcbcrConversionCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSamplerYcbcrConversion* pYcbcrConversion) const {
	return _table.vkCreateSamplerYcbcrConversion(_device, pCreateInfo, pAllocator, pYcbcrConversion);
}
inline VkResult Create(const VkSamplerYcbcrConversionCreateInfo* pCreateInfo, VkSamplerYcbcrConversion* pYcbcrConversion) const {
	return _table.vkCreateSamplerYcbcrConversion(_device, pCreateInfo, _allocator, pYcbcrConversion);
}
inline VkResult Create(const VkSamplerYcbcrConversionCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSamplerYcbcrConversion* pYcbcrConversion) const {
	return _table.vkCreateSamplerYcbcrConversion(_device, pCreateInfo, pAllocator, pYcbcrConversion);
}
inline VkResult CreateSamplerYcbcrConversion(const VkSamplerYcbcrConversionCreateInfo* pCreateInfo, VkSamplerYcbcrConversion* pYcbcrConversion) const {
	return _table.vkCreateSamplerYcbcrConversion(_device, pCreateInfo, _allocator, pYcbcrConversion);
}
inline void Destroy(VkDescriptorUpdateTemplate descriptorUpdateTemplate, const VkAllocationCallbacks* pAllocator) const {
	_table.vkDestroyDescriptorUpdateTemplate(_device, descriptorUpdateTemplate, pAllocator);
}
inline void DestroyDescriptorUpdateTemplate(VkDescriptorUpdateTemplate descriptorUpdateTemplate) const {
	_table.vkDestroyDescriptorUpdateTemplate(_device, descriptorUpdateTemplate, _allocator);
}
inline void DestroyDescriptorUpdateTemplate(VkDescriptorUpdateTemplate descriptorUpdateTemplate, const VkAllocationCallbacks* pAllocator) const {
	_table.vkDestroyDescriptorUpdateTemplate(_device, descriptorUpdateTemplate, pAllocator);
}
inline void Destroy(VkDescriptorUpdateTemplate descriptorUpdateTemplate) const {
	_table.vkDestroyDescriptorUpdateTemplate(_device, descriptorUpdateTemplate, _allocator);
}
inline void Destroy(VkSamplerYcbcrConversion ycbcrConversion, const VkAllocationCallbacks* pAllocator) const {
	_table.vkDestroySamplerYcbcrConversion(_device, ycbcrConversion, pAllocator);
}
inline void DestroySamplerYcbcrConversion(VkSamplerYcbcrConversion ycbcrConversion) const {
	_table.vkDestroySamplerYcbcrConversion(_device, ycbcrConversion, _allocator);
}
inline void DestroySamplerYcbcrConversion(VkSamplerYcbcrConversion ycbcrConversion, const VkAllocationCallbacks* pAllocator) const {
	_table.vkDestroySamplerYcbcrConversion(_device, ycbcrConversion, pAllocator);
}
inline void Destroy(VkSamplerYcbcrConversion ycbcrConversion) const {
	_table.vkDestroySamplerYcbcrConversion(_device, ycbcrConversion, _allocator);
}
inline void GetBufferMemoryRequirements2(const VkBufferMemoryRequirementsInfo2* pInfo, VkMemoryRequirements2* pMemoryRequirements) const {
	_table.vkGetBufferMemoryRequirements2(_device, pInfo, pMemoryRequirements);
}
inline void GetDescriptorSetLayoutSupport(const VkDescriptorSetLayoutCreateInfo* pCreateInfo, VkDescriptorSetLayoutSupport* pSupport) const {
	_table.vkGetDescriptorSetLayoutSupport(_device, pCreateInfo, pSupport);
}
inline void GetDeviceGroupPeerMemoryFeatures(uint32_t heapIndex, uint32_t localDeviceIndex, uint32_t remoteDeviceIndex, VkPeerMemoryFeatureFlags* pPeerMemoryFeatures) const {
	_table.vkGetDeviceGroupPeerMemoryFeatures(_device, heapIndex, localDeviceIndex, remoteDeviceIndex, pPeerMemoryFeatures);
}
inline void GetDeviceQueue2(const VkDeviceQueueInfo2* pQueueInfo, VkQueue* pQueue) const {
	_table.vkGetDeviceQueue2(_device, pQueueInfo, pQueue);
}
inline void GetImageMemoryRequirements2(const VkImageMemoryRequirementsInfo2* pInfo, VkMemoryRequirements2* pMemoryRequirements) const {
	_table.vkGetImageMemoryRequirements2(_device, pInfo, pMemoryRequirements);
}
inline void GetImageSparseMemoryRequirements2(const VkImageSparseMemoryRequirementsInfo2* pInfo, uint32_t* pSparseMemoryRequirementCount, VkSparseImageMemoryRequirements2* pSparseMemoryRequirements) const {
	_table.vkGetImageSparseMemoryRequirements2(_device, pInfo, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
}
inline void TrimCommandPool(VkCommandPool commandPool, VkCommandPoolTrimFlags flags) const {
	_table.vkTrimCommandPool(_device, commandPool, flags);
}
inline void UpdateDescriptorSetWithTemplate(VkDescriptorSet descriptorSet, VkDescriptorUpdateTemplate descriptorUpdateTemplate, const void* pData) const {
	_table.vkUpdateDescriptorSetWithTemplate(_device, descriptorSet, descriptorUpdateTemplate, pData);
}
#endif /* defined(VK_VERSION_1_1) */
#if defined(VK_VERSION_1_2)
inline void CmdBeginRenderPass2(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo*      pRenderPassBegin, const VkSubpassBeginInfo*      pSubpassBeginInfo) const {
	_table.vkCmdBeginRenderPass2(commandBuffer, pRenderPassBegin, pSubpassBeginInfo);
}
inline void CmdDrawIndexedIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride) const {
	_table.vkCmdDrawIndexedIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
}
inline void CmdDrawIndirectCount(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride) const {
	_table.vkCmdDrawIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
}
inline void CmdEndRenderPass2(VkCommandBuffer commandBuffer, const VkSubpassEndInfo*        pSubpassEndInfo) const {
	_table.vkCmdEndRenderPass2(commandBuffer, pSubpassEndInfo);
}
inline void CmdNextSubpass2(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo*      pSubpassBeginInfo, const VkSubpassEndInfo*        pSubpassEndInfo) const {
	_table.vkCmdNextSubpass2(commandBuffer, pSubpassBeginInfo, pSubpassEndInfo);
}
inline VkResult Create(const VkRenderPassCreateInfo2* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass) const {
	return _table.vkCreateRenderPass2(_device, pCreateInfo, pAllocator, pRenderPass);
}
inline VkResult CreateRenderPass2(const VkRenderPassCreateInfo2* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass) const {
	return _table.vkCreateRenderPass2(_device, pCreateInfo, pAllocator, pRenderPass);
}
inline VkResult Create(const VkRenderPassCreateInfo2* pCreateInfo, VkRenderPass* pRenderPass) const {
	return _table.vkCreateRenderPass2(_device, pCreateInfo, _allocator, pRenderPass);
}
inline VkResult CreateRenderPass2(const VkRenderPassCreateInfo2* pCreateInfo, VkRenderPass* pRenderPass) const {
	return _table.vkCreateRenderPass2(_device, pCreateInfo, _allocator, pRenderPass);
}
inline VkDeviceAddress GetBufferDeviceAddress(const VkBufferDeviceAddressInfo* pInfo) const {
	return _table.vkGetBufferDeviceAddress(_device, pInfo);
}
inline uint64_t GetBufferOpaqueCaptureAddress(const VkBufferDeviceAddressInfo* pInfo) const {
	return _table.vkGetBufferOpaqueCaptureAddress(_device, pInfo);
}
inline uint64_t GetDeviceMemoryOpaqueCaptureAddress(const VkDeviceMemoryOpaqueCaptureAddressInfo* pInfo) const {
	return _table.vkGetDeviceMemoryOpaqueCaptureAddress(_device, pInfo);
}
inline VkResult GetSemaphoreCounterValue(VkSemaphore semaphore, uint64_t* pValue) const {
	return _table.vkGetSemaphoreCounterValue(_device, semaphore, pValue);
}
inline void ResetQueryPool(VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount) const {
	_table.vkResetQueryPool(_device, queryPool, firstQuery, queryCount);
}
inline VkResult SignalSemaphore(const VkSemaphoreSignalInfo* pSignalInfo) const {
	return _table.vkSignalSemaphore(_device, pSignalInfo);
}
inline VkResult WaitSemaphores(const VkSemaphoreWaitInfo* pWaitInfo, uint64_t timeout) const {
	return _table.vkWaitSemaphores(_device, pWaitInfo, timeout);
}
#endif /* defined(VK_VERSION_1_2) */
#if defined(VK_VERSION_1_3)
inline void CmdBeginRendering(VkCommandBuffer                   commandBuffer, const VkRenderingInfo*                              pRenderingInfo) const {
	_table.vkCmdBeginRendering(commandBuffer, pRenderingInfo);
}
inline void CmdBindVertexBuffers2(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount, const VkBuffer* pBuffers, const VkDeviceSize* pOffsets, const VkDeviceSize* pSizes, const VkDeviceSize* pStrides) const {
	_table.vkCmdBindVertexBuffers2(commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets, pSizes, pStrides);
}
inline void CmdBlitImage2(VkCommandBuffer commandBuffer, const VkBlitImageInfo2* pBlitImageInfo) const {
	_table.vkCmdBlitImage2(commandBuffer, pBlitImageInfo);
}
inline void CmdCopyBuffer2(VkCommandBuffer commandBuffer, const VkCopyBufferInfo2* pCopyBufferInfo) const {
	_table.vkCmdCopyBuffer2(commandBuffer, pCopyBufferInfo);
}
inline void CmdCopyBufferToImage2(VkCommandBuffer commandBuffer, const VkCopyBufferToImageInfo2* pCopyBufferToImageInfo) const {
	_table.vkCmdCopyBufferToImage2(commandBuffer, pCopyBufferToImageInfo);
}
inline void CmdCopyImage2(VkCommandBuffer commandBuffer, const VkCopyImageInfo2* pCopyImageInfo) const {
	_table.vkCmdCopyImage2(commandBuffer, pCopyImageInfo);
}
inline void CmdCopyImageToBuffer2(VkCommandBuffer commandBuffer, const VkCopyImageToBufferInfo2* pCopyImageToBufferInfo) const {
	_table.vkCmdCopyImageToBuffer2(commandBuffer, pCopyImageToBufferInfo);
}
inline void CmdEndRendering(VkCommandBuffer                   commandBuffer) const {
	_table.vkCmdEndRendering(commandBuffer);
}
inline void CmdPipelineBarrier2(VkCommandBuffer                   commandBuffer, const VkDependencyInfo*                             pDependencyInfo) const {
	_table.vkCmdPipelineBarrier2(commandBuffer, pDependencyInfo);
}
inline void CmdResetEvent2(VkCommandBuffer                   commandBuffer, VkEvent                                             event, VkPipelineStageFlags2               stageMask) const {
	_table.vkCmdResetEvent2(commandBuffer, event, stageMask);
}
inline void CmdResolveImage2(VkCommandBuffer commandBuffer, const VkResolveImageInfo2* pResolveImageInfo) const {
	_table.vkCmdResolveImage2(commandBuffer, pResolveImageInfo);
}
inline void CmdSetCullMode(VkCommandBuffer commandBuffer, VkCullModeFlags cullMode) const {
	_table.vkCmdSetCullMode(commandBuffer, cullMode);
}
inline void CmdSetDepthBiasEnable(VkCommandBuffer commandBuffer, VkBool32 depthBiasEnable) const {
	_table.vkCmdSetDepthBiasEnable(commandBuffer, depthBiasEnable);
}
inline void CmdSetDepthBoundsTestEnable(VkCommandBuffer commandBuffer, VkBool32 depthBoundsTestEnable) const {
	_table.vkCmdSetDepthBoundsTestEnable(commandBuffer, depthBoundsTestEnable);
}
inline void CmdSetDepthCompareOp(VkCommandBuffer commandBuffer, VkCompareOp depthCompareOp) const {
	_table.vkCmdSetDepthCompareOp(commandBuffer, depthCompareOp);
}
inline void CmdSetDepthTestEnable(VkCommandBuffer commandBuffer, VkBool32 depthTestEnable) const {
	_table.vkCmdSetDepthTestEnable(commandBuffer, depthTestEnable);
}
inline void CmdSetDepthWriteEnable(VkCommandBuffer commandBuffer, VkBool32 depthWriteEnable) const {
	_table.vkCmdSetDepthWriteEnable(commandBuffer, depthWriteEnable);
}
inline void CmdSetEvent2(VkCommandBuffer                   commandBuffer, VkEvent                                             event, const VkDependencyInfo*                             pDependencyInfo) const {
	_table.vkCmdSetEvent2(commandBuffer, event, pDependencyInfo);
}
inline void CmdSetFrontFace(VkCommandBuffer commandBuffer, VkFrontFace frontFace) const {
	_table.vkCmdSetFrontFace(commandBuffer, frontFace);
}
inline void CmdSetPrimitiveRestartEnable(VkCommandBuffer commandBuffer, VkBool32 primitiveRestartEnable) const {
	_table.vkCmdSetPrimitiveRestartEnable(commandBuffer, primitiveRestartEnable);
}
inline void CmdSetPrimitiveTopology(VkCommandBuffer commandBuffer, VkPrimitiveTopology primitiveTopology) const {
	_table.vkCmdSetPrimitiveTopology(commandBuffer, primitiveTopology);
}
inline void CmdSetRasterizerDiscardEnable(VkCommandBuffer commandBuffer, VkBool32 rasterizerDiscardEnable) const {
	_table.vkCmdSetRasterizerDiscardEnable(commandBuffer, rasterizerDiscardEnable);
}
inline void CmdSetScissorWithCount(VkCommandBuffer commandBuffer, uint32_t scissorCount, const VkRect2D* pScissors) const {
	_table.vkCmdSetScissorWithCount(commandBuffer, scissorCount, pScissors);
}
inline void CmdSetStencilOp(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, VkStencilOp failOp, VkStencilOp passOp, VkStencilOp depthFailOp, VkCompareOp compareOp) const {
	_table.vkCmdSetStencilOp(commandBuffer, faceMask, failOp, passOp, depthFailOp, compareOp);
}
inline void CmdSetStencilTestEnable(VkCommandBuffer commandBuffer, VkBool32 stencilTestEnable) const {
	_table.vkCmdSetStencilTestEnable(commandBuffer, stencilTestEnable);
}
inline void CmdSetViewportWithCount(VkCommandBuffer commandBuffer, uint32_t viewportCount, const VkViewport* pViewports) const {
	_table.vkCmdSetViewportWithCount(commandBuffer, viewportCount, pViewports);
}
inline void CmdWaitEvents2(VkCommandBuffer                   commandBuffer, uint32_t                                            eventCount, const VkEvent*                     pEvents, const VkDependencyInfo*            pDependencyInfos) const {
	_table.vkCmdWaitEvents2(commandBuffer, eventCount, pEvents, pDependencyInfos);
}
inline void CmdWriteTimestamp2(VkCommandBuffer                   commandBuffer, VkPipelineStageFlags2               stage, VkQueryPool                                         queryPool, uint32_t                                            query) const {
	_table.vkCmdWriteTimestamp2(commandBuffer, stage, queryPool, query);
}
inline VkResult Create(const VkPrivateDataSlotCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkPrivateDataSlot* pPrivateDataSlot) const {
	return _table.vkCreatePrivateDataSlot(_device, pCreateInfo, pAllocator, pPrivateDataSlot);
}
inline VkResult CreatePrivateDataSlot(const VkPrivateDataSlotCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkPrivateDataSlot* pPrivateDataSlot) const {
	return _table.vkCreatePrivateDataSlot(_device, pCreateInfo, pAllocator, pPrivateDataSlot);
}
inline VkResult Create(const VkPrivateDataSlotCreateInfo* pCreateInfo, VkPrivateDataSlot* pPrivateDataSlot) const {
	return _table.vkCreatePrivateDataSlot(_device, pCreateInfo, _allocator, pPrivateDataSlot);
}
inline VkResult CreatePrivateDataSlot(const VkPrivateDataSlotCreateInfo* pCreateInfo, VkPrivateDataSlot* pPrivateDataSlot) const {
	return _table.vkCreatePrivateDataSlot(_device, pCreateInfo, _allocator, pPrivateDataSlot);
}
inline void Destroy(VkPrivateDataSlot privateDataSlot, const VkAllocationCallbacks* pAllocator) const {
	_table.vkDestroyPrivateDataSlot(_device, privateDataSlot, pAllocator);
}
inline void Destroy(VkPrivateDataSlot privateDataSlot) const {
	_table.vkDestroyPrivateDataSlot(_device, privateDataSlot, _allocator);
}
inline void DestroyPrivateDataSlot(VkPrivateDataSlot privateDataSlot) const {
	_table.vkDestroyPrivateDataSlot(_device, privateDataSlot, _allocator);
}
inline void DestroyPrivateDataSlot(VkPrivateDataSlot privateDataSlot, const VkAllocationCallbacks* pAllocator) const {
	_table.vkDestroyPrivateDataSlot(_device, privateDataSlot, pAllocator);
}
inline void GetDeviceBufferMemoryRequirements(const VkDeviceBufferMemoryRequirements* pInfo, VkMemoryRequirements2* pMemoryRequirements) const {
	_table.vkGetDeviceBufferMemoryRequirements(_device, pInfo, pMemoryRequirements);
}
inline void GetDeviceImageMemoryRequirements(const VkDeviceImageMemoryRequirements* pInfo, VkMemoryRequirements2* pMemoryRequirements) const {
	_table.vkGetDeviceImageMemoryRequirements(_device, pInfo, pMemoryRequirements);
}
inline void GetDeviceImageSparseMemoryRequirements(const VkDeviceImageMemoryRequirements* pInfo, uint32_t* pSparseMemoryRequirementCount, VkSparseImageMemoryRequirements2* pSparseMemoryRequirements) const {
	_table.vkGetDeviceImageSparseMemoryRequirements(_device, pInfo, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
}
inline void GetPrivateData(VkObjectType objectType, uint64_t objectHandle, VkPrivateDataSlot privateDataSlot, uint64_t* pData) const {
	_table.vkGetPrivateData(_device, objectType, objectHandle, privateDataSlot, pData);
}
inline VkResult QueueSubmit2(VkQueue                           queue, uint32_t                            submitCount, const VkSubmitInfo2*              pSubmits, VkFence           fence) const {
	return _table.vkQueueSubmit2(queue, submitCount, pSubmits, fence);
}
inline VkResult SetPrivateData(VkObjectType objectType, uint64_t objectHandle, VkPrivateDataSlot privateDataSlot, uint64_t data) const {
	return _table.vkSetPrivateData(_device, objectType, objectHandle, privateDataSlot, data);
}
#endif /* defined(VK_VERSION_1_3) */
#if defined(VK_AMD_buffer_marker)
inline void CmdWriteBufferMarkerAMD(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage, VkBuffer dstBuffer, VkDeviceSize dstOffset, uint32_t marker) const {
	_table.vkCmdWriteBufferMarkerAMD(commandBuffer, pipelineStage, dstBuffer, dstOffset, marker);
}
#endif /* defined(VK_AMD_buffer_marker) */
#if defined(VK_AMD_display_native_hdr)
inline void SetLocalDimmingAMD(VkSwapchainKHR swapChain, VkBool32 localDimmingEnable) const {
	_table.vkSetLocalDimmingAMD(_device, swapChain, localDimmingEnable);
}
#endif /* defined(VK_AMD_display_native_hdr) */
#if defined(VK_AMD_draw_indirect_count)
inline void CmdDrawIndexedIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride) const {
	_table.vkCmdDrawIndexedIndirectCountAMD(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
}
inline void CmdDrawIndirectCountAMD(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride) const {
	_table.vkCmdDrawIndirectCountAMD(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
}
#endif /* defined(VK_AMD_draw_indirect_count) */
#if defined(VK_AMD_shader_info)
inline VkResult GetShaderInfoAMD(VkPipeline pipeline, VkShaderStageFlagBits shaderStage, VkShaderInfoTypeAMD infoType, size_t* pInfoSize, void* pInfo) const {
	return _table.vkGetShaderInfoAMD(_device, pipeline, shaderStage, infoType, pInfoSize, pInfo);
}
#endif /* defined(VK_AMD_shader_info) */
#if defined(VK_ANDROID_external_memory_android_hardware_buffer)
inline VkResult GetAndroidHardwareBufferPropertiesANDROID(const struct AHardwareBuffer* buffer, VkAndroidHardwareBufferPropertiesANDROID* pProperties) const {
	return _table.vkGetAndroidHardwareBufferPropertiesANDROID(_device, buffer, pProperties);
}
inline VkResult GetMemoryAndroidHardwareBufferANDROID(const VkMemoryGetAndroidHardwareBufferInfoANDROID* pInfo, struct AHardwareBuffer** pBuffer) const {
	return _table.vkGetMemoryAndroidHardwareBufferANDROID(_device, pInfo, pBuffer);
}
#endif /* defined(VK_ANDROID_external_memory_android_hardware_buffer) */
#if defined(VK_EXT_buffer_device_address)
inline VkDeviceAddress GetBufferDeviceAddressEXT(const VkBufferDeviceAddressInfo* pInfo) const {
	return _table.vkGetBufferDeviceAddressEXT(_device, pInfo);
}
#endif /* defined(VK_EXT_buffer_device_address) */
#if defined(VK_EXT_calibrated_timestamps)
inline VkResult GetCalibratedTimestampsEXT(uint32_t timestampCount, const VkCalibratedTimestampInfoEXT* pTimestampInfos, uint64_t* pTimestamps, uint64_t* pMaxDeviation) const {
	return _table.vkGetCalibratedTimestampsEXT(_device, timestampCount, pTimestampInfos, pTimestamps, pMaxDeviation);
}
#endif /* defined(VK_EXT_calibrated_timestamps) */
#if defined(VK_EXT_color_write_enable)
inline void CmdSetColorWriteEnableEXT(VkCommandBuffer       commandBuffer, uint32_t                                attachmentCount, const VkBool32*   pColorWriteEnables) const {
	_table.vkCmdSetColorWriteEnableEXT(commandBuffer, attachmentCount, pColorWriteEnables);
}
#endif /* defined(VK_EXT_color_write_enable) */
#if defined(VK_EXT_conditional_rendering)
inline void CmdBeginConditionalRenderingEXT(VkCommandBuffer commandBuffer, const VkConditionalRenderingBeginInfoEXT* pConditionalRenderingBegin) const {
	_table.vkCmdBeginConditionalRenderingEXT(commandBuffer, pConditionalRenderingBegin);
}
inline void CmdEndConditionalRenderingEXT(VkCommandBuffer commandBuffer) const {
	_table.vkCmdEndConditionalRenderingEXT(commandBuffer);
}
#endif /* defined(VK_EXT_conditional_rendering) */
#if defined(VK_EXT_debug_marker)
inline void CmdDebugMarkerBeginEXT(VkCommandBuffer commandBuffer, const VkDebugMarkerMarkerInfoEXT* pMarkerInfo) const {
	_table.vkCmdDebugMarkerBeginEXT(commandBuffer, pMarkerInfo);
}
inline void CmdDebugMarkerEndEXT(VkCommandBuffer commandBuffer) const {
	_table.vkCmdDebugMarkerEndEXT(commandBuffer);
}
inline void CmdDebugMarkerInsertEXT(VkCommandBuffer commandBuffer, const VkDebugMarkerMarkerInfoEXT* pMarkerInfo) const {
	_table.vkCmdDebugMarkerInsertEXT(commandBuffer, pMarkerInfo);
}
inline VkResult DebugMarkerSetObjectNameEXT(const VkDebugMarkerObjectNameInfoEXT* pNameInfo) const {
	return _table.vkDebugMarkerSetObjectNameEXT(_device, pNameInfo);
}
inline VkResult DebugMarkerSetObjectTagEXT(const VkDebugMarkerObjectTagInfoEXT* pTagInfo) const {
	return _table.vkDebugMarkerSetObjectTagEXT(_device, pTagInfo);
}
#endif /* defined(VK_EXT_debug_marker) */
#if defined(VK_EXT_discard_rectangles)
inline void CmdSetDiscardRectangleEXT(VkCommandBuffer commandBuffer, uint32_t firstDiscardRectangle, uint32_t discardRectangleCount, const VkRect2D* pDiscardRectangles) const {
	_table.vkCmdSetDiscardRectangleEXT(commandBuffer, firstDiscardRectangle, discardRectangleCount, pDiscardRectangles);
}
#endif /* defined(VK_EXT_discard_rectangles) */
#if defined(VK_EXT_display_control)
inline VkResult DisplayPowerControlEXT(VkDisplayKHR display, const VkDisplayPowerInfoEXT* pDisplayPowerInfo) const {
	return _table.vkDisplayPowerControlEXT(_device, display, pDisplayPowerInfo);
}
inline VkResult GetSwapchainCounterEXT(VkSwapchainKHR swapchain, VkSurfaceCounterFlagBitsEXT counter, uint64_t* pCounterValue) const {
	return _table.vkGetSwapchainCounterEXT(_device, swapchain, counter, pCounterValue);
}
inline VkResult RegisterDeviceEventEXT(const VkDeviceEventInfoEXT* pDeviceEventInfo, const VkAllocationCallbacks* pAllocator, VkFence* pFence) const {
	return _table.vkRegisterDeviceEventEXT(_device, pDeviceEventInfo, pAllocator, pFence);
}
inline VkResult RegisterDeviceEventEXT(const VkDeviceEventInfoEXT* pDeviceEventInfo, VkFence* pFence) const {
	return _table.vkRegisterDeviceEventEXT(_device, pDeviceEventInfo, _allocator, pFence);
}
inline VkResult RegisterDisplayEventEXT(VkDisplayKHR display, const VkDisplayEventInfoEXT* pDisplayEventInfo, const VkAllocationCallbacks* pAllocator, VkFence* pFence) const {
	return _table.vkRegisterDisplayEventEXT(_device, display, pDisplayEventInfo, pAllocator, pFence);
}
inline VkResult RegisterDisplayEventEXT(VkDisplayKHR display, const VkDisplayEventInfoEXT* pDisplayEventInfo, VkFence* pFence) const {
	return _table.vkRegisterDisplayEventEXT(_device, display, pDisplayEventInfo, _allocator, pFence);
}
#endif /* defined(VK_EXT_display_control) */
#if defined(VK_EXT_extended_dynamic_state)
inline void CmdBindVertexBuffers2EXT(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount, const VkBuffer* pBuffers, const VkDeviceSize* pOffsets, const VkDeviceSize* pSizes, const VkDeviceSize* pStrides) const {
	_table.vkCmdBindVertexBuffers2EXT(commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets, pSizes, pStrides);
}
inline void CmdSetCullModeEXT(VkCommandBuffer commandBuffer, VkCullModeFlags cullMode) const {
	_table.vkCmdSetCullModeEXT(commandBuffer, cullMode);
}
inline void CmdSetDepthBoundsTestEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthBoundsTestEnable) const {
	_table.vkCmdSetDepthBoundsTestEnableEXT(commandBuffer, depthBoundsTestEnable);
}
inline void CmdSetDepthCompareOpEXT(VkCommandBuffer commandBuffer, VkCompareOp depthCompareOp) const {
	_table.vkCmdSetDepthCompareOpEXT(commandBuffer, depthCompareOp);
}
inline void CmdSetDepthTestEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthTestEnable) const {
	_table.vkCmdSetDepthTestEnableEXT(commandBuffer, depthTestEnable);
}
inline void CmdSetDepthWriteEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthWriteEnable) const {
	_table.vkCmdSetDepthWriteEnableEXT(commandBuffer, depthWriteEnable);
}
inline void CmdSetFrontFaceEXT(VkCommandBuffer commandBuffer, VkFrontFace frontFace) const {
	_table.vkCmdSetFrontFaceEXT(commandBuffer, frontFace);
}
inline void CmdSetPrimitiveTopologyEXT(VkCommandBuffer commandBuffer, VkPrimitiveTopology primitiveTopology) const {
	_table.vkCmdSetPrimitiveTopologyEXT(commandBuffer, primitiveTopology);
}
inline void CmdSetScissorWithCountEXT(VkCommandBuffer commandBuffer, uint32_t scissorCount, const VkRect2D* pScissors) const {
	_table.vkCmdSetScissorWithCountEXT(commandBuffer, scissorCount, pScissors);
}
inline void CmdSetStencilOpEXT(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, VkStencilOp failOp, VkStencilOp passOp, VkStencilOp depthFailOp, VkCompareOp compareOp) const {
	_table.vkCmdSetStencilOpEXT(commandBuffer, faceMask, failOp, passOp, depthFailOp, compareOp);
}
inline void CmdSetStencilTestEnableEXT(VkCommandBuffer commandBuffer, VkBool32 stencilTestEnable) const {
	_table.vkCmdSetStencilTestEnableEXT(commandBuffer, stencilTestEnable);
}
inline void CmdSetViewportWithCountEXT(VkCommandBuffer commandBuffer, uint32_t viewportCount, const VkViewport* pViewports) const {
	_table.vkCmdSetViewportWithCountEXT(commandBuffer, viewportCount, pViewports);
}
#endif /* defined(VK_EXT_extended_dynamic_state) */
#if defined(VK_EXT_extended_dynamic_state2)
inline void CmdSetDepthBiasEnableEXT(VkCommandBuffer commandBuffer, VkBool32 depthBiasEnable) const {
	_table.vkCmdSetDepthBiasEnableEXT(commandBuffer, depthBiasEnable);
}
inline void CmdSetLogicOpEXT(VkCommandBuffer commandBuffer, VkLogicOp logicOp) const {
	_table.vkCmdSetLogicOpEXT(commandBuffer, logicOp);
}
inline void CmdSetPatchControlPointsEXT(VkCommandBuffer commandBuffer, uint32_t patchControlPoints) const {
	_table.vkCmdSetPatchControlPointsEXT(commandBuffer, patchControlPoints);
}
inline void CmdSetPrimitiveRestartEnableEXT(VkCommandBuffer commandBuffer, VkBool32 primitiveRestartEnable) const {
	_table.vkCmdSetPrimitiveRestartEnableEXT(commandBuffer, primitiveRestartEnable);
}
inline void CmdSetRasterizerDiscardEnableEXT(VkCommandBuffer commandBuffer, VkBool32 rasterizerDiscardEnable) const {
	_table.vkCmdSetRasterizerDiscardEnableEXT(commandBuffer, rasterizerDiscardEnable);
}
#endif /* defined(VK_EXT_extended_dynamic_state2) */
#if defined(VK_EXT_external_memory_host)
inline VkResult GetMemoryHostPointerPropertiesEXT(VkExternalMemoryHandleTypeFlagBits handleType, const void* pHostPointer, VkMemoryHostPointerPropertiesEXT* pMemoryHostPointerProperties) const {
	return _table.vkGetMemoryHostPointerPropertiesEXT(_device, handleType, pHostPointer, pMemoryHostPointerProperties);
}
#endif /* defined(VK_EXT_external_memory_host) */
#if defined(VK_EXT_full_screen_exclusive)
inline VkResult AcquireFullScreenExclusiveModeEXT(VkSwapchainKHR swapchain) const {
	return _table.vkAcquireFullScreenExclusiveModeEXT(_device, swapchain);
}
inline VkResult ReleaseFullScreenExclusiveModeEXT(VkSwapchainKHR swapchain) const {
	return _table.vkReleaseFullScreenExclusiveModeEXT(_device, swapchain);
}
#endif /* defined(VK_EXT_full_screen_exclusive) */
#if defined(VK_EXT_hdr_metadata)
inline void SetHdrMetadataEXT(uint32_t swapchainCount, const VkSwapchainKHR* pSwapchains, const VkHdrMetadataEXT* pMetadata) const {
	_table.vkSetHdrMetadataEXT(_device, swapchainCount, pSwapchains, pMetadata);
}
#endif /* defined(VK_EXT_hdr_metadata) */
#if defined(VK_EXT_host_query_reset)
inline void ResetQueryPoolEXT(VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount) const {
	_table.vkResetQueryPoolEXT(_device, queryPool, firstQuery, queryCount);
}
#endif /* defined(VK_EXT_host_query_reset) */
#if defined(VK_EXT_image_compression_control)
inline void GetImageSubresourceLayout2EXT(VkImage image, const VkImageSubresource2EXT* pSubresource, VkSubresourceLayout2EXT* pLayout) const {
	_table.vkGetImageSubresourceLayout2EXT(_device, image, pSubresource, pLayout);
}
#endif /* defined(VK_EXT_image_compression_control) */
#if defined(VK_EXT_image_drm_format_modifier)
inline VkResult GetImageDrmFormatModifierPropertiesEXT(VkImage image, VkImageDrmFormatModifierPropertiesEXT* pProperties) const {
	return _table.vkGetImageDrmFormatModifierPropertiesEXT(_device, image, pProperties);
}
#endif /* defined(VK_EXT_image_drm_format_modifier) */
#if defined(VK_EXT_line_rasterization)
inline void CmdSetLineStippleEXT(VkCommandBuffer commandBuffer, uint32_t lineStippleFactor, uint16_t lineStipplePattern) const {
	_table.vkCmdSetLineStippleEXT(commandBuffer, lineStippleFactor, lineStipplePattern);
}
#endif /* defined(VK_EXT_line_rasterization) */
#if defined(VK_EXT_metal_objects)
inline void ExportMetalObjectsEXT(VkExportMetalObjectsInfoEXT* pMetalObjectsInfo) const {
	_table.vkExportMetalObjectsEXT(_device, pMetalObjectsInfo);
}
#endif /* defined(VK_EXT_metal_objects) */
#if defined(VK_EXT_multi_draw)
inline void CmdDrawMultiEXT(VkCommandBuffer commandBuffer, uint32_t drawCount, const VkMultiDrawInfoEXT* pVertexInfo, uint32_t instanceCount, uint32_t firstInstance, uint32_t stride) const {
	_table.vkCmdDrawMultiEXT(commandBuffer, drawCount, pVertexInfo, instanceCount, firstInstance, stride);
}
inline void CmdDrawMultiIndexedEXT(VkCommandBuffer commandBuffer, uint32_t drawCount, const VkMultiDrawIndexedInfoEXT* pIndexInfo, uint32_t instanceCount, uint32_t firstInstance, uint32_t stride, const int32_t* pVertexOffset) const {
	_table.vkCmdDrawMultiIndexedEXT(commandBuffer, drawCount, pIndexInfo, instanceCount, firstInstance, stride, pVertexOffset);
}
#endif /* defined(VK_EXT_multi_draw) */
#if defined(VK_EXT_pageable_device_local_memory)
inline void SetDeviceMemoryPriorityEXT(VkDeviceMemory memory, float          priority) const {
	_table.vkSetDeviceMemoryPriorityEXT(_device, memory, priority);
}
#endif /* defined(VK_EXT_pageable_device_local_memory) */
#if defined(VK_EXT_pipeline_properties)
inline VkResult GetPipelinePropertiesEXT(const VkPipelineInfoEXT* pPipelineInfo, VkBaseOutStructure* pPipelineProperties) const {
	return _table.vkGetPipelinePropertiesEXT(_device, pPipelineInfo, pPipelineProperties);
}
#endif /* defined(VK_EXT_pipeline_properties) */
#if defined(VK_EXT_private_data)
#if !defined(VK_VERSION_1_3)
inline VkResult CreatePrivateDataSlotEXT(const VkPrivateDataSlotCreateInfo* pCreateInfo, VkPrivateDataSlot* pPrivateDataSlot) const {
	return _table.vkCreatePrivateDataSlotEXT(_device, pCreateInfo, _allocator, pPrivateDataSlot);
}
#endif
#if !defined(VK_VERSION_1_3)
inline VkResult CreatePrivateDataSlotEXT(const VkPrivateDataSlotCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkPrivateDataSlot* pPrivateDataSlot) const {
	return _table.vkCreatePrivateDataSlotEXT(_device, pCreateInfo, pAllocator, pPrivateDataSlot);
}
#endif
#if !defined(VK_VERSION_1_3)
inline VkResult Create(const VkPrivateDataSlotCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkPrivateDataSlot* pPrivateDataSlot) const {
	return _table.vkCreatePrivateDataSlotEXT(_device, pCreateInfo, pAllocator, pPrivateDataSlot);
}
#endif
#if !defined(VK_VERSION_1_3)
inline VkResult Create(const VkPrivateDataSlotCreateInfo* pCreateInfo, VkPrivateDataSlot* pPrivateDataSlot) const {
	return _table.vkCreatePrivateDataSlotEXT(_device, pCreateInfo, _allocator, pPrivateDataSlot);
}
#endif
#if !defined(VK_VERSION_1_3)
inline void DestroyPrivateDataSlotEXT(VkPrivateDataSlot privateDataSlot) const {
	_table.vkDestroyPrivateDataSlotEXT(_device, privateDataSlot, _allocator);
}
#endif
#if !defined(VK_VERSION_1_3)
inline void Destroy(VkPrivateDataSlot privateDataSlot) const {
	_table.vkDestroyPrivateDataSlotEXT(_device, privateDataSlot, _allocator);
}
#endif
#if !defined(VK_VERSION_1_3)
inline void Destroy(VkPrivateDataSlot privateDataSlot, const VkAllocationCallbacks* pAllocator) const {
	_table.vkDestroyPrivateDataSlotEXT(_device, privateDataSlot, pAllocator);
}
#endif
#if !defined(VK_VERSION_1_3)
inline void DestroyPrivateDataSlotEXT(VkPrivateDataSlot privateDataSlot, const VkAllocationCallbacks* pAllocator) const {
	_table.vkDestroyPrivateDataSlotEXT(_device, privateDataSlot, pAllocator);
}
#endif
inline void GetPrivateDataEXT(VkObjectType objectType, uint64_t objectHandle, VkPrivateDataSlot privateDataSlot, uint64_t* pData) const {
	_table.vkGetPrivateDataEXT(_device, objectType, objectHandle, privateDataSlot, pData);
}
inline VkResult SetPrivateDataEXT(VkObjectType objectType, uint64_t objectHandle, VkPrivateDataSlot privateDataSlot, uint64_t data) const {
	return _table.vkSetPrivateDataEXT(_device, objectType, objectHandle, privateDataSlot, data);
}
#endif /* defined(VK_EXT_private_data) */
#if defined(VK_EXT_sample_locations)
inline void CmdSetSampleLocationsEXT(VkCommandBuffer commandBuffer, const VkSampleLocationsInfoEXT* pSampleLocationsInfo) const {
	_table.vkCmdSetSampleLocationsEXT(commandBuffer, pSampleLocationsInfo);
}
#endif /* defined(VK_EXT_sample_locations) */
#if defined(VK_EXT_shader_module_identifier)
inline void GetShaderModuleCreateInfoIdentifierEXT(const VkShaderModuleCreateInfo* pCreateInfo, VkShaderModuleIdentifierEXT* pIdentifier) const {
	_table.vkGetShaderModuleCreateInfoIdentifierEXT(_device, pCreateInfo, pIdentifier);
}
inline void GetShaderModuleIdentifierEXT(VkShaderModule shaderModule, VkShaderModuleIdentifierEXT* pIdentifier) const {
	_table.vkGetShaderModuleIdentifierEXT(_device, shaderModule, pIdentifier);
}
#endif /* defined(VK_EXT_shader_module_identifier) */
#if defined(VK_EXT_transform_feedback)
inline void CmdBeginQueryIndexedEXT(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query, VkQueryControlFlags flags, uint32_t index) const {
	_table.vkCmdBeginQueryIndexedEXT(commandBuffer, queryPool, query, flags, index);
}
inline void CmdBeginTransformFeedbackEXT(VkCommandBuffer commandBuffer, uint32_t firstCounterBuffer, uint32_t counterBufferCount, const VkBuffer* pCounterBuffers, const VkDeviceSize* pCounterBufferOffsets) const {
	_table.vkCmdBeginTransformFeedbackEXT(commandBuffer, firstCounterBuffer, counterBufferCount, pCounterBuffers, pCounterBufferOffsets);
}
inline void CmdBindTransformFeedbackBuffersEXT(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount, const VkBuffer* pBuffers, const VkDeviceSize* pOffsets, const VkDeviceSize* pSizes) const {
	_table.vkCmdBindTransformFeedbackBuffersEXT(commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets, pSizes);
}
inline void CmdDrawIndirectByteCountEXT(VkCommandBuffer commandBuffer, uint32_t instanceCount, uint32_t firstInstance, VkBuffer counterBuffer, VkDeviceSize counterBufferOffset, uint32_t counterOffset, uint32_t vertexStride) const {
	_table.vkCmdDrawIndirectByteCountEXT(commandBuffer, instanceCount, firstInstance, counterBuffer, counterBufferOffset, counterOffset, vertexStride);
}
inline void CmdEndQueryIndexedEXT(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query, uint32_t index) const {
	_table.vkCmdEndQueryIndexedEXT(commandBuffer, queryPool, query, index);
}
inline void CmdEndTransformFeedbackEXT(VkCommandBuffer commandBuffer, uint32_t firstCounterBuffer, uint32_t counterBufferCount, const VkBuffer* pCounterBuffers, const VkDeviceSize* pCounterBufferOffsets) const {
	_table.vkCmdEndTransformFeedbackEXT(commandBuffer, firstCounterBuffer, counterBufferCount, pCounterBuffers, pCounterBufferOffsets);
}
#endif /* defined(VK_EXT_transform_feedback) */
#if defined(VK_EXT_validation_cache)
inline VkResult CreateValidationCacheEXT(const VkValidationCacheCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkValidationCacheEXT* pValidationCache) const {
	return _table.vkCreateValidationCacheEXT(_device, pCreateInfo, pAllocator, pValidationCache);
}
inline VkResult Create(const VkValidationCacheCreateInfoEXT* pCreateInfo, VkValidationCacheEXT* pValidationCache) const {
	return _table.vkCreateValidationCacheEXT(_device, pCreateInfo, _allocator, pValidationCache);
}
inline VkResult CreateValidationCacheEXT(const VkValidationCacheCreateInfoEXT* pCreateInfo, VkValidationCacheEXT* pValidationCache) const {
	return _table.vkCreateValidationCacheEXT(_device, pCreateInfo, _allocator, pValidationCache);
}
inline VkResult Create(const VkValidationCacheCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkValidationCacheEXT* pValidationCache) const {
	return _table.vkCreateValidationCacheEXT(_device, pCreateInfo, pAllocator, pValidationCache);
}
inline void Destroy(VkValidationCacheEXT validationCache) const {
	_table.vkDestroyValidationCacheEXT(_device, validationCache, _allocator);
}
inline void DestroyValidationCacheEXT(VkValidationCacheEXT validationCache, const VkAllocationCallbacks* pAllocator) const {
	_table.vkDestroyValidationCacheEXT(_device, validationCache, pAllocator);
}
inline void DestroyValidationCacheEXT(VkValidationCacheEXT validationCache) const {
	_table.vkDestroyValidationCacheEXT(_device, validationCache, _allocator);
}
inline void Destroy(VkValidationCacheEXT validationCache, const VkAllocationCallbacks* pAllocator) const {
	_table.vkDestroyValidationCacheEXT(_device, validationCache, pAllocator);
}
inline VkResult GetValidationCacheDataEXT(VkValidationCacheEXT validationCache, size_t* pDataSize, void* pData) const {
	return _table.vkGetValidationCacheDataEXT(_device, validationCache, pDataSize, pData);
}
inline VkResult MergeValidationCachesEXT(VkValidationCacheEXT dstCache, uint32_t srcCacheCount, const VkValidationCacheEXT* pSrcCaches) const {
	return _table.vkMergeValidationCachesEXT(_device, dstCache, srcCacheCount, pSrcCaches);
}
#endif /* defined(VK_EXT_validation_cache) */
#if defined(VK_EXT_vertex_input_dynamic_state)
inline void CmdSetVertexInputEXT(VkCommandBuffer commandBuffer, uint32_t vertexBindingDescriptionCount, const VkVertexInputBindingDescription2EXT* pVertexBindingDescriptions, uint32_t vertexAttributeDescriptionCount, const VkVertexInputAttributeDescription2EXT* pVertexAttributeDescriptions) const {
	_table.vkCmdSetVertexInputEXT(commandBuffer, vertexBindingDescriptionCount, pVertexBindingDescriptions, vertexAttributeDescriptionCount, pVertexAttributeDescriptions);
}
#endif /* defined(VK_EXT_vertex_input_dynamic_state) */
#if defined(VK_FUCHSIA_buffer_collection)
inline VkResult Create(const VkBufferCollectionCreateInfoFUCHSIA* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkBufferCollectionFUCHSIA* pCollection) const {
	return _table.vkCreateBufferCollectionFUCHSIA(_device, pCreateInfo, pAllocator, pCollection);
}
inline VkResult CreateBufferCollectionFUCHSIA(const VkBufferCollectionCreateInfoFUCHSIA* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkBufferCollectionFUCHSIA* pCollection) const {
	return _table.vkCreateBufferCollectionFUCHSIA(_device, pCreateInfo, pAllocator, pCollection);
}
inline VkResult Create(const VkBufferCollectionCreateInfoFUCHSIA* pCreateInfo, VkBufferCollectionFUCHSIA* pCollection) const {
	return _table.vkCreateBufferCollectionFUCHSIA(_device, pCreateInfo, _allocator, pCollection);
}
inline VkResult CreateBufferCollectionFUCHSIA(const VkBufferCollectionCreateInfoFUCHSIA* pCreateInfo, VkBufferCollectionFUCHSIA* pCollection) const {
	return _table.vkCreateBufferCollectionFUCHSIA(_device, pCreateInfo, _allocator, pCollection);
}
inline void DestroyBufferCollectionFUCHSIA(VkBufferCollectionFUCHSIA collection, const VkAllocationCallbacks* pAllocator) const {
	_table.vkDestroyBufferCollectionFUCHSIA(_device, collection, pAllocator);
}
inline void DestroyBufferCollectionFUCHSIA(VkBufferCollectionFUCHSIA collection) const {
	_table.vkDestroyBufferCollectionFUCHSIA(_device, collection, _allocator);
}
inline void Destroy(VkBufferCollectionFUCHSIA collection, const VkAllocationCallbacks* pAllocator) const {
	_table.vkDestroyBufferCollectionFUCHSIA(_device, collection, pAllocator);
}
inline void Destroy(VkBufferCollectionFUCHSIA collection) const {
	_table.vkDestroyBufferCollectionFUCHSIA(_device, collection, _allocator);
}
inline VkResult GetBufferCollectionPropertiesFUCHSIA(VkBufferCollectionFUCHSIA collection, VkBufferCollectionPropertiesFUCHSIA* pProperties) const {
	return _table.vkGetBufferCollectionPropertiesFUCHSIA(_device, collection, pProperties);
}
inline VkResult SetBufferCollectionBufferConstraintsFUCHSIA(VkBufferCollectionFUCHSIA collection, const VkBufferConstraintsInfoFUCHSIA* pBufferConstraintsInfo) const {
	return _table.vkSetBufferCollectionBufferConstraintsFUCHSIA(_device, collection, pBufferConstraintsInfo);
}
inline VkResult SetBufferCollectionImageConstraintsFUCHSIA(VkBufferCollectionFUCHSIA collection, const VkImageConstraintsInfoFUCHSIA* pImageConstraintsInfo) const {
	return _table.vkSetBufferCollectionImageConstraintsFUCHSIA(_device, collection, pImageConstraintsInfo);
}
#endif /* defined(VK_FUCHSIA_buffer_collection) */
#if defined(VK_FUCHSIA_external_memory)
inline VkResult GetMemoryZirconHandleFUCHSIA(const VkMemoryGetZirconHandleInfoFUCHSIA* pGetZirconHandleInfo, zx_handle_t* pZirconHandle) const {
	return _table.vkGetMemoryZirconHandleFUCHSIA(_device, pGetZirconHandleInfo, pZirconHandle);
}
inline VkResult GetMemoryZirconHandlePropertiesFUCHSIA(VkExternalMemoryHandleTypeFlagBits handleType, zx_handle_t zirconHandle, VkMemoryZirconHandlePropertiesFUCHSIA* pMemoryZirconHandleProperties) const {
	return _table.vkGetMemoryZirconHandlePropertiesFUCHSIA(_device, handleType, zirconHandle, pMemoryZirconHandleProperties);
}
#endif /* defined(VK_FUCHSIA_external_memory) */
#if defined(VK_FUCHSIA_external_semaphore)
inline VkResult GetSemaphoreZirconHandleFUCHSIA(const VkSemaphoreGetZirconHandleInfoFUCHSIA* pGetZirconHandleInfo, zx_handle_t* pZirconHandle) const {
	return _table.vkGetSemaphoreZirconHandleFUCHSIA(_device, pGetZirconHandleInfo, pZirconHandle);
}
inline VkResult ImportSemaphoreZirconHandleFUCHSIA(const VkImportSemaphoreZirconHandleInfoFUCHSIA* pImportSemaphoreZirconHandleInfo) const {
	return _table.vkImportSemaphoreZirconHandleFUCHSIA(_device, pImportSemaphoreZirconHandleInfo);
}
#endif /* defined(VK_FUCHSIA_external_semaphore) */
#if defined(VK_GOOGLE_display_timing)
inline VkResult GetPastPresentationTimingGOOGLE(VkSwapchainKHR swapchain, uint32_t* pPresentationTimingCount, VkPastPresentationTimingGOOGLE* pPresentationTimings) const {
	return _table.vkGetPastPresentationTimingGOOGLE(_device, swapchain, pPresentationTimingCount, pPresentationTimings);
}
inline VkResult GetRefreshCycleDurationGOOGLE(VkSwapchainKHR swapchain, VkRefreshCycleDurationGOOGLE* pDisplayTimingProperties) const {
	return _table.vkGetRefreshCycleDurationGOOGLE(_device, swapchain, pDisplayTimingProperties);
}
#endif /* defined(VK_GOOGLE_display_timing) */
#if defined(VK_HUAWEI_invocation_mask)
inline void CmdBindInvocationMaskHUAWEI(VkCommandBuffer commandBuffer, VkImageView imageView, VkImageLayout imageLayout) const {
	_table.vkCmdBindInvocationMaskHUAWEI(commandBuffer, imageView, imageLayout);
}
#endif /* defined(VK_HUAWEI_invocation_mask) */
#if defined(VK_HUAWEI_subpass_shading)
inline void CmdSubpassShadingHUAWEI(VkCommandBuffer commandBuffer) const {
	_table.vkCmdSubpassShadingHUAWEI(commandBuffer);
}
inline VkResult GetDeviceSubpassShadingMaxWorkgroupSizeHUAWEI(VkRenderPass renderpass, VkExtent2D* pMaxWorkgroupSize) const {
	return _table.vkGetDeviceSubpassShadingMaxWorkgroupSizeHUAWEI(_device, renderpass, pMaxWorkgroupSize);
}
#endif /* defined(VK_HUAWEI_subpass_shading) */
#if defined(VK_INTEL_performance_query)
inline VkResult AcquirePerformanceConfigurationINTEL(const VkPerformanceConfigurationAcquireInfoINTEL* pAcquireInfo, VkPerformanceConfigurationINTEL* pConfiguration) const {
	return _table.vkAcquirePerformanceConfigurationINTEL(_device, pAcquireInfo, pConfiguration);
}
inline VkResult CmdSetPerformanceMarkerINTEL(VkCommandBuffer commandBuffer, const VkPerformanceMarkerInfoINTEL* pMarkerInfo) const {
	return _table.vkCmdSetPerformanceMarkerINTEL(commandBuffer, pMarkerInfo);
}
inline VkResult CmdSetPerformanceOverrideINTEL(VkCommandBuffer commandBuffer, const VkPerformanceOverrideInfoINTEL* pOverrideInfo) const {
	return _table.vkCmdSetPerformanceOverrideINTEL(commandBuffer, pOverrideInfo);
}
inline VkResult CmdSetPerformanceStreamMarkerINTEL(VkCommandBuffer commandBuffer, const VkPerformanceStreamMarkerInfoINTEL* pMarkerInfo) const {
	return _table.vkCmdSetPerformanceStreamMarkerINTEL(commandBuffer, pMarkerInfo);
}
inline VkResult GetPerformanceParameterINTEL(VkPerformanceParameterTypeINTEL parameter, VkPerformanceValueINTEL* pValue) const {
	return _table.vkGetPerformanceParameterINTEL(_device, parameter, pValue);
}
inline VkResult InitializePerformanceApiINTEL(const VkInitializePerformanceApiInfoINTEL* pInitializeInfo) const {
	return _table.vkInitializePerformanceApiINTEL(_device, pInitializeInfo);
}
inline VkResult QueueSetPerformanceConfigurationINTEL(VkQueue queue, VkPerformanceConfigurationINTEL configuration) const {
	return _table.vkQueueSetPerformanceConfigurationINTEL(queue, configuration);
}
inline VkResult ReleasePerformanceConfigurationINTEL(VkPerformanceConfigurationINTEL configuration) const {
	return _table.vkReleasePerformanceConfigurationINTEL(_device, configuration);
}
inline void UninitializePerformanceApiINTEL() const {
	_table.vkUninitializePerformanceApiINTEL(_device);
}
#endif /* defined(VK_INTEL_performance_query) */
#if defined(VK_KHR_acceleration_structure)
inline VkResult BuildAccelerationStructuresKHR(VkDeferredOperationKHR deferredOperation, uint32_t infoCount, const VkAccelerationStructureBuildGeometryInfoKHR* pInfos, const VkAccelerationStructureBuildRangeInfoKHR* const* ppBuildRangeInfos) const {
	return _table.vkBuildAccelerationStructuresKHR(_device, deferredOperation, infoCount, pInfos, ppBuildRangeInfos);
}
inline void CmdBuildAccelerationStructuresIndirectKHR(VkCommandBuffer                  commandBuffer, uint32_t                                           infoCount, const VkAccelerationStructureBuildGeometryInfoKHR* pInfos, const VkDeviceAddress*             pIndirectDeviceAddresses, const uint32_t*                    pIndirectStrides, const uint32_t* const*             ppMaxPrimitiveCounts) const {
	_table.vkCmdBuildAccelerationStructuresIndirectKHR(commandBuffer, infoCount, pInfos, pIndirectDeviceAddresses, pIndirectStrides, ppMaxPrimitiveCounts);
}
inline void CmdBuildAccelerationStructuresKHR(VkCommandBuffer                                    commandBuffer, uint32_t infoCount, const VkAccelerationStructureBuildGeometryInfoKHR* pInfos, const VkAccelerationStructureBuildRangeInfoKHR* const* ppBuildRangeInfos) const {
	_table.vkCmdBuildAccelerationStructuresKHR(commandBuffer, infoCount, pInfos, ppBuildRangeInfos);
}
inline void CmdCopyAccelerationStructureKHR(VkCommandBuffer commandBuffer, const VkCopyAccelerationStructureInfoKHR* pInfo) const {
	_table.vkCmdCopyAccelerationStructureKHR(commandBuffer, pInfo);
}
inline void CmdCopyAccelerationStructureToMemoryKHR(VkCommandBuffer commandBuffer, const VkCopyAccelerationStructureToMemoryInfoKHR* pInfo) const {
	_table.vkCmdCopyAccelerationStructureToMemoryKHR(commandBuffer, pInfo);
}
inline void CmdCopyMemoryToAccelerationStructureKHR(VkCommandBuffer commandBuffer, const VkCopyMemoryToAccelerationStructureInfoKHR* pInfo) const {
	_table.vkCmdCopyMemoryToAccelerationStructureKHR(commandBuffer, pInfo);
}
inline void CmdWriteAccelerationStructuresPropertiesKHR(VkCommandBuffer commandBuffer, uint32_t accelerationStructureCount, const VkAccelerationStructureKHR* pAccelerationStructures, VkQueryType queryType, VkQueryPool queryPool, uint32_t firstQuery) const {
	_table.vkCmdWriteAccelerationStructuresPropertiesKHR(commandBuffer, accelerationStructureCount, pAccelerationStructures, queryType, queryPool, firstQuery);
}
inline VkResult CopyAccelerationStructureKHR(VkDeferredOperationKHR deferredOperation, const VkCopyAccelerationStructureInfoKHR* pInfo) const {
	return _table.vkCopyAccelerationStructureKHR(_device, deferredOperation, pInfo);
}
inline VkResult CopyAccelerationStructureToMemoryKHR(VkDeferredOperationKHR deferredOperation, const VkCopyAccelerationStructureToMemoryInfoKHR* pInfo) const {
	return _table.vkCopyAccelerationStructureToMemoryKHR(_device, deferredOperation, pInfo);
}
inline VkResult CopyMemoryToAccelerationStructureKHR(VkDeferredOperationKHR deferredOperation, const VkCopyMemoryToAccelerationStructureInfoKHR* pInfo) const {
	return _table.vkCopyMemoryToAccelerationStructureKHR(_device, deferredOperation, pInfo);
}
inline VkResult Create(const VkAccelerationStructureCreateInfoKHR*        pCreateInfo, const VkAllocationCallbacks*       pAllocator, VkAccelerationStructureKHR*                        pAccelerationStructure) const {
	return _table.vkCreateAccelerationStructureKHR(_device, pCreateInfo, pAllocator, pAccelerationStructure);
}
inline VkResult CreateAccelerationStructureKHR(const VkAccelerationStructureCreateInfoKHR*        pCreateInfo, VkAccelerationStructureKHR*                        pAccelerationStructure) const {
	return _table.vkCreateAccelerationStructureKHR(_device, pCreateInfo, _allocator, pAccelerationStructure);
}
inline VkResult Create(const VkAccelerationStructureCreateInfoKHR*        pCreateInfo, VkAccelerationStructureKHR*                        pAccelerationStructure) const {
	return _table.vkCreateAccelerationStructureKHR(_device, pCreateInfo, _allocator, pAccelerationStructure);
}
inline VkResult CreateAccelerationStructureKHR(const VkAccelerationStructureCreateInfoKHR*        pCreateInfo, const VkAllocationCallbacks*       pAllocator, VkAccelerationStructureKHR*                        pAccelerationStructure) const {
	return _table.vkCreateAccelerationStructureKHR(_device, pCreateInfo, pAllocator, pAccelerationStructure);
}
inline void Destroy(VkAccelerationStructureKHR accelerationStructure) const {
	_table.vkDestroyAccelerationStructureKHR(_device, accelerationStructure, _allocator);
}
inline void Destroy(VkAccelerationStructureKHR accelerationStructure, const VkAllocationCallbacks* pAllocator) const {
	_table.vkDestroyAccelerationStructureKHR(_device, accelerationStructure, pAllocator);
}
inline void DestroyAccelerationStructureKHR(VkAccelerationStructureKHR accelerationStructure) const {
	_table.vkDestroyAccelerationStructureKHR(_device, accelerationStructure, _allocator);
}
inline void DestroyAccelerationStructureKHR(VkAccelerationStructureKHR accelerationStructure, const VkAllocationCallbacks* pAllocator) const {
	_table.vkDestroyAccelerationStructureKHR(_device, accelerationStructure, pAllocator);
}
inline void GetAccelerationStructureBuildSizesKHR(VkAccelerationStructureBuildTypeKHR                 buildType, const VkAccelerationStructureBuildGeometryInfoKHR*  pBuildInfo, const uint32_t*  pMaxPrimitiveCounts, VkAccelerationStructureBuildSizesInfoKHR*           pSizeInfo) const {
	_table.vkGetAccelerationStructureBuildSizesKHR(_device, buildType, pBuildInfo, pMaxPrimitiveCounts, pSizeInfo);
}
inline VkDeviceAddress GetAccelerationStructureDeviceAddressKHR(const VkAccelerationStructureDeviceAddressInfoKHR* pInfo) const {
	return _table.vkGetAccelerationStructureDeviceAddressKHR(_device, pInfo);
}
inline void GetDeviceAccelerationStructureCompatibilityKHR(const VkAccelerationStructureVersionInfoKHR* pVersionInfo, VkAccelerationStructureCompatibilityKHR* pCompatibility) const {
	_table.vkGetDeviceAccelerationStructureCompatibilityKHR(_device, pVersionInfo, pCompatibility);
}
inline VkResult WriteAccelerationStructuresPropertiesKHR(uint32_t accelerationStructureCount, const VkAccelerationStructureKHR* pAccelerationStructures, VkQueryType  queryType, size_t       dataSize, void* pData, size_t stride) const {
	return _table.vkWriteAccelerationStructuresPropertiesKHR(_device, accelerationStructureCount, pAccelerationStructures, queryType, dataSize, pData, stride);
}
#endif /* defined(VK_KHR_acceleration_structure) */
#if defined(VK_KHR_bind_memory2)
inline VkResult BindBufferMemory2KHR(uint32_t bindInfoCount, const VkBindBufferMemoryInfo* pBindInfos) const {
	return _table.vkBindBufferMemory2KHR(_device, bindInfoCount, pBindInfos);
}
inline VkResult BindImageMemory2KHR(uint32_t bindInfoCount, const VkBindImageMemoryInfo* pBindInfos) const {
	return _table.vkBindImageMemory2KHR(_device, bindInfoCount, pBindInfos);
}
#endif /* defined(VK_KHR_bind_memory2) */
#if defined(VK_KHR_buffer_device_address)
inline VkDeviceAddress GetBufferDeviceAddressKHR(const VkBufferDeviceAddressInfo* pInfo) const {
	return _table.vkGetBufferDeviceAddressKHR(_device, pInfo);
}
inline uint64_t GetBufferOpaqueCaptureAddressKHR(const VkBufferDeviceAddressInfo* pInfo) const {
	return _table.vkGetBufferOpaqueCaptureAddressKHR(_device, pInfo);
}
inline uint64_t GetDeviceMemoryOpaqueCaptureAddressKHR(const VkDeviceMemoryOpaqueCaptureAddressInfo* pInfo) const {
	return _table.vkGetDeviceMemoryOpaqueCaptureAddressKHR(_device, pInfo);
}
#endif /* defined(VK_KHR_buffer_device_address) */
#if defined(VK_KHR_copy_commands2)
inline void CmdBlitImage2KHR(VkCommandBuffer commandBuffer, const VkBlitImageInfo2* pBlitImageInfo) const {
	_table.vkCmdBlitImage2KHR(commandBuffer, pBlitImageInfo);
}
inline void CmdCopyBuffer2KHR(VkCommandBuffer commandBuffer, const VkCopyBufferInfo2* pCopyBufferInfo) const {
	_table.vkCmdCopyBuffer2KHR(commandBuffer, pCopyBufferInfo);
}
inline void CmdCopyBufferToImage2KHR(VkCommandBuffer commandBuffer, const VkCopyBufferToImageInfo2* pCopyBufferToImageInfo) const {
	_table.vkCmdCopyBufferToImage2KHR(commandBuffer, pCopyBufferToImageInfo);
}
inline void CmdCopyImage2KHR(VkCommandBuffer commandBuffer, const VkCopyImageInfo2* pCopyImageInfo) const {
	_table.vkCmdCopyImage2KHR(commandBuffer, pCopyImageInfo);
}
inline void CmdCopyImageToBuffer2KHR(VkCommandBuffer commandBuffer, const VkCopyImageToBufferInfo2* pCopyImageToBufferInfo) const {
	_table.vkCmdCopyImageToBuffer2KHR(commandBuffer, pCopyImageToBufferInfo);
}
inline void CmdResolveImage2KHR(VkCommandBuffer commandBuffer, const VkResolveImageInfo2* pResolveImageInfo) const {
	_table.vkCmdResolveImage2KHR(commandBuffer, pResolveImageInfo);
}
#endif /* defined(VK_KHR_copy_commands2) */
#if defined(VK_KHR_create_renderpass2)
inline void CmdBeginRenderPass2KHR(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo*      pRenderPassBegin, const VkSubpassBeginInfo*      pSubpassBeginInfo) const {
	_table.vkCmdBeginRenderPass2KHR(commandBuffer, pRenderPassBegin, pSubpassBeginInfo);
}
inline void CmdEndRenderPass2KHR(VkCommandBuffer commandBuffer, const VkSubpassEndInfo*        pSubpassEndInfo) const {
	_table.vkCmdEndRenderPass2KHR(commandBuffer, pSubpassEndInfo);
}
inline void CmdNextSubpass2KHR(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo*      pSubpassBeginInfo, const VkSubpassEndInfo*        pSubpassEndInfo) const {
	_table.vkCmdNextSubpass2KHR(commandBuffer, pSubpassBeginInfo, pSubpassEndInfo);
}
#if !defined(VK_VERSION_1_2)
inline VkResult CreateRenderPass2KHR(const VkRenderPassCreateInfo2* pCreateInfo, VkRenderPass* pRenderPass) const {
	return _table.vkCreateRenderPass2KHR(_device, pCreateInfo, _allocator, pRenderPass);
}
#endif
#if !defined(VK_VERSION_1_2)
inline VkResult Create(const VkRenderPassCreateInfo2* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass) const {
	return _table.vkCreateRenderPass2KHR(_device, pCreateInfo, pAllocator, pRenderPass);
}
#endif
#if !defined(VK_VERSION_1_2)
inline VkResult Create(const VkRenderPassCreateInfo2* pCreateInfo, VkRenderPass* pRenderPass) const {
	return _table.vkCreateRenderPass2KHR(_device, pCreateInfo, _allocator, pRenderPass);
}
#endif
#if !defined(VK_VERSION_1_2)
inline VkResult CreateRenderPass2KHR(const VkRenderPassCreateInfo2* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass) const {
	return _table.vkCreateRenderPass2KHR(_device, pCreateInfo, pAllocator, pRenderPass);
}
#endif
#endif /* defined(VK_KHR_create_renderpass2) */
#if defined(VK_KHR_deferred_host_operations)
inline VkResult CreateDeferredOperationKHR(const VkAllocationCallbacks* pAllocator, VkDeferredOperationKHR* pDeferredOperation) const {
	return _table.vkCreateDeferredOperationKHR(_device, pAllocator, pDeferredOperation);
}
inline VkResult Create(const VkAllocationCallbacks* pAllocator, VkDeferredOperationKHR* pDeferredOperation) const {
	return _table.vkCreateDeferredOperationKHR(_device, pAllocator, pDeferredOperation);
}
inline VkResult Create(VkDeferredOperationKHR* pDeferredOperation) const {
	return _table.vkCreateDeferredOperationKHR(_device, _allocator, pDeferredOperation);
}
inline VkResult CreateDeferredOperationKHR(VkDeferredOperationKHR* pDeferredOperation) const {
	return _table.vkCreateDeferredOperationKHR(_device, _allocator, pDeferredOperation);
}
inline VkResult DeferredOperationJoinKHR(VkDeferredOperationKHR operation) const {
	return _table.vkDeferredOperationJoinKHR(_device, operation);
}
inline void Destroy(VkDeferredOperationKHR operation) const {
	_table.vkDestroyDeferredOperationKHR(_device, operation, _allocator);
}
inline void Destroy(VkDeferredOperationKHR operation, const VkAllocationCallbacks* pAllocator) const {
	_table.vkDestroyDeferredOperationKHR(_device, operation, pAllocator);
}
inline void DestroyDeferredOperationKHR(VkDeferredOperationKHR operation) const {
	_table.vkDestroyDeferredOperationKHR(_device, operation, _allocator);
}
inline void DestroyDeferredOperationKHR(VkDeferredOperationKHR operation, const VkAllocationCallbacks* pAllocator) const {
	_table.vkDestroyDeferredOperationKHR(_device, operation, pAllocator);
}
inline uint32_t GetDeferredOperationMaxConcurrencyKHR(VkDeferredOperationKHR operation) const {
	return _table.vkGetDeferredOperationMaxConcurrencyKHR(_device, operation);
}
inline VkResult GetDeferredOperationResultKHR(VkDeferredOperationKHR operation) const {
	return _table.vkGetDeferredOperationResultKHR(_device, operation);
}
#endif /* defined(VK_KHR_deferred_host_operations) */
#if defined(VK_KHR_descriptor_update_template)
#if !defined(VK_VERSION_1_1)
inline VkResult Create(const VkDescriptorUpdateTemplateCreateInfo* pCreateInfo, VkDescriptorUpdateTemplate* pDescriptorUpdateTemplate) const {
	return _table.vkCreateDescriptorUpdateTemplateKHR(_device, pCreateInfo, _allocator, pDescriptorUpdateTemplate);
}
#endif
#if !defined(VK_VERSION_1_1)
inline VkResult Create(const VkDescriptorUpdateTemplateCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDescriptorUpdateTemplate* pDescriptorUpdateTemplate) const {
	return _table.vkCreateDescriptorUpdateTemplateKHR(_device, pCreateInfo, pAllocator, pDescriptorUpdateTemplate);
}
#endif
#if !defined(VK_VERSION_1_1)
inline VkResult CreateDescriptorUpdateTemplateKHR(const VkDescriptorUpdateTemplateCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDescriptorUpdateTemplate* pDescriptorUpdateTemplate) const {
	return _table.vkCreateDescriptorUpdateTemplateKHR(_device, pCreateInfo, pAllocator, pDescriptorUpdateTemplate);
}
#endif
#if !defined(VK_VERSION_1_1)
inline VkResult CreateDescriptorUpdateTemplateKHR(const VkDescriptorUpdateTemplateCreateInfo* pCreateInfo, VkDescriptorUpdateTemplate* pDescriptorUpdateTemplate) const {
	return _table.vkCreateDescriptorUpdateTemplateKHR(_device, pCreateInfo, _allocator, pDescriptorUpdateTemplate);
}
#endif
#if !defined(VK_VERSION_1_1)
inline void DestroyDescriptorUpdateTemplateKHR(VkDescriptorUpdateTemplate descriptorUpdateTemplate, const VkAllocationCallbacks* pAllocator) const {
	_table.vkDestroyDescriptorUpdateTemplateKHR(_device, descriptorUpdateTemplate, pAllocator);
}
#endif
#if !defined(VK_VERSION_1_1)
inline void DestroyDescriptorUpdateTemplateKHR(VkDescriptorUpdateTemplate descriptorUpdateTemplate) const {
	_table.vkDestroyDescriptorUpdateTemplateKHR(_device, descriptorUpdateTemplate, _allocator);
}
#endif
#if !defined(VK_VERSION_1_1)
inline void Destroy(VkDescriptorUpdateTemplate descriptorUpdateTemplate, const VkAllocationCallbacks* pAllocator) const {
	_table.vkDestroyDescriptorUpdateTemplateKHR(_device, descriptorUpdateTemplate, pAllocator);
}
#endif
#if !defined(VK_VERSION_1_1)
inline void Destroy(VkDescriptorUpdateTemplate descriptorUpdateTemplate) const {
	_table.vkDestroyDescriptorUpdateTemplateKHR(_device, descriptorUpdateTemplate, _allocator);
}
#endif
inline void UpdateDescriptorSetWithTemplateKHR(VkDescriptorSet descriptorSet, VkDescriptorUpdateTemplate descriptorUpdateTemplate, const void* pData) const {
	_table.vkUpdateDescriptorSetWithTemplateKHR(_device, descriptorSet, descriptorUpdateTemplate, pData);
}
#endif /* defined(VK_KHR_descriptor_update_template) */
#if defined(VK_KHR_device_group)
inline void CmdDispatchBaseKHR(VkCommandBuffer commandBuffer, uint32_t baseGroupX, uint32_t baseGroupY, uint32_t baseGroupZ, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) const {
	_table.vkCmdDispatchBaseKHR(commandBuffer, baseGroupX, baseGroupY, baseGroupZ, groupCountX, groupCountY, groupCountZ);
}
inline void CmdSetDeviceMaskKHR(VkCommandBuffer commandBuffer, uint32_t deviceMask) const {
	_table.vkCmdSetDeviceMaskKHR(commandBuffer, deviceMask);
}
inline void GetDeviceGroupPeerMemoryFeaturesKHR(uint32_t heapIndex, uint32_t localDeviceIndex, uint32_t remoteDeviceIndex, VkPeerMemoryFeatureFlags* pPeerMemoryFeatures) const {
	_table.vkGetDeviceGroupPeerMemoryFeaturesKHR(_device, heapIndex, localDeviceIndex, remoteDeviceIndex, pPeerMemoryFeatures);
}
#endif /* defined(VK_KHR_device_group) */
#if defined(VK_KHR_display_swapchain)
inline VkResult CreateSharedSwapchainsKHR(uint32_t swapchainCount, const VkSwapchainCreateInfoKHR* pCreateInfos, VkSwapchainKHR* pSwapchains) const {
	return _table.vkCreateSharedSwapchainsKHR(_device, swapchainCount, pCreateInfos, _allocator, pSwapchains);
}
inline VkResult Create(uint32_t swapchainCount, const VkSwapchainCreateInfoKHR* pCreateInfos, VkSwapchainKHR* pSwapchains) const {
	return _table.vkCreateSharedSwapchainsKHR(_device, swapchainCount, pCreateInfos, _allocator, pSwapchains);
}
inline VkResult Create(uint32_t swapchainCount, const VkSwapchainCreateInfoKHR* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchains) const {
	return _table.vkCreateSharedSwapchainsKHR(_device, swapchainCount, pCreateInfos, pAllocator, pSwapchains);
}
inline VkResult CreateSharedSwapchainsKHR(uint32_t swapchainCount, const VkSwapchainCreateInfoKHR* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchains) const {
	return _table.vkCreateSharedSwapchainsKHR(_device, swapchainCount, pCreateInfos, pAllocator, pSwapchains);
}
#endif /* defined(VK_KHR_display_swapchain) */
#if defined(VK_KHR_draw_indirect_count)
inline void CmdDrawIndexedIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride) const {
	_table.vkCmdDrawIndexedIndirectCountKHR(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
}
inline void CmdDrawIndirectCountKHR(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride) const {
	_table.vkCmdDrawIndirectCountKHR(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
}
#endif /* defined(VK_KHR_draw_indirect_count) */
#if defined(VK_KHR_dynamic_rendering)
inline void CmdBeginRenderingKHR(VkCommandBuffer                   commandBuffer, const VkRenderingInfo*                              pRenderingInfo) const {
	_table.vkCmdBeginRenderingKHR(commandBuffer, pRenderingInfo);
}
inline void CmdEndRenderingKHR(VkCommandBuffer                   commandBuffer) const {
	_table.vkCmdEndRenderingKHR(commandBuffer);
}
#endif /* defined(VK_KHR_dynamic_rendering) */
#if defined(VK_KHR_external_fence_fd)
inline VkResult GetFenceFdKHR(const VkFenceGetFdInfoKHR* pGetFdInfo, int* pFd) const {
	return _table.vkGetFenceFdKHR(_device, pGetFdInfo, pFd);
}
inline VkResult ImportFenceFdKHR(const VkImportFenceFdInfoKHR* pImportFenceFdInfo) const {
	return _table.vkImportFenceFdKHR(_device, pImportFenceFdInfo);
}
#endif /* defined(VK_KHR_external_fence_fd) */
#if defined(VK_KHR_external_fence_win32)
inline VkResult GetFenceWin32HandleKHR(const VkFenceGetWin32HandleInfoKHR* pGetWin32HandleInfo, HANDLE* pHandle) const {
	return _table.vkGetFenceWin32HandleKHR(_device, pGetWin32HandleInfo, pHandle);
}
inline VkResult ImportFenceWin32HandleKHR(const VkImportFenceWin32HandleInfoKHR* pImportFenceWin32HandleInfo) const {
	return _table.vkImportFenceWin32HandleKHR(_device, pImportFenceWin32HandleInfo);
}
#endif /* defined(VK_KHR_external_fence_win32) */
#if defined(VK_KHR_external_memory_fd)
inline VkResult GetMemoryFdKHR(const VkMemoryGetFdInfoKHR* pGetFdInfo, int* pFd) const {
	return _table.vkGetMemoryFdKHR(_device, pGetFdInfo, pFd);
}
inline VkResult GetMemoryFdPropertiesKHR(VkExternalMemoryHandleTypeFlagBits handleType, int fd, VkMemoryFdPropertiesKHR* pMemoryFdProperties) const {
	return _table.vkGetMemoryFdPropertiesKHR(_device, handleType, fd, pMemoryFdProperties);
}
#endif /* defined(VK_KHR_external_memory_fd) */
#if defined(VK_KHR_external_memory_win32)
inline VkResult GetMemoryWin32HandleKHR(const VkMemoryGetWin32HandleInfoKHR* pGetWin32HandleInfo, HANDLE* pHandle) const {
	return _table.vkGetMemoryWin32HandleKHR(_device, pGetWin32HandleInfo, pHandle);
}
inline VkResult GetMemoryWin32HandlePropertiesKHR(VkExternalMemoryHandleTypeFlagBits handleType, HANDLE handle, VkMemoryWin32HandlePropertiesKHR* pMemoryWin32HandleProperties) const {
	return _table.vkGetMemoryWin32HandlePropertiesKHR(_device, handleType, handle, pMemoryWin32HandleProperties);
}
#endif /* defined(VK_KHR_external_memory_win32) */
#if defined(VK_KHR_external_semaphore_fd)
inline VkResult GetSemaphoreFdKHR(const VkSemaphoreGetFdInfoKHR* pGetFdInfo, int* pFd) const {
	return _table.vkGetSemaphoreFdKHR(_device, pGetFdInfo, pFd);
}
inline VkResult ImportSemaphoreFdKHR(const VkImportSemaphoreFdInfoKHR* pImportSemaphoreFdInfo) const {
	return _table.vkImportSemaphoreFdKHR(_device, pImportSemaphoreFdInfo);
}
#endif /* defined(VK_KHR_external_semaphore_fd) */
#if defined(VK_KHR_external_semaphore_win32)
inline VkResult GetSemaphoreWin32HandleKHR(const VkSemaphoreGetWin32HandleInfoKHR* pGetWin32HandleInfo, HANDLE* pHandle) const {
	return _table.vkGetSemaphoreWin32HandleKHR(_device, pGetWin32HandleInfo, pHandle);
}
inline VkResult ImportSemaphoreWin32HandleKHR(const VkImportSemaphoreWin32HandleInfoKHR* pImportSemaphoreWin32HandleInfo) const {
	return _table.vkImportSemaphoreWin32HandleKHR(_device, pImportSemaphoreWin32HandleInfo);
}
#endif /* defined(VK_KHR_external_semaphore_win32) */
#if defined(VK_KHR_fragment_shading_rate)
inline void CmdSetFragmentShadingRateKHR(VkCommandBuffer           commandBuffer, const VkExtent2D*                           pFragmentSize, const VkFragmentShadingRateCombinerOpKHR    combinerOps[2]) const {
	_table.vkCmdSetFragmentShadingRateKHR(commandBuffer, pFragmentSize, combinerOps);
}
#endif /* defined(VK_KHR_fragment_shading_rate) */
#if defined(VK_KHR_get_memory_requirements2)
inline void GetBufferMemoryRequirements2KHR(const VkBufferMemoryRequirementsInfo2* pInfo, VkMemoryRequirements2* pMemoryRequirements) const {
	_table.vkGetBufferMemoryRequirements2KHR(_device, pInfo, pMemoryRequirements);
}
inline void GetImageMemoryRequirements2KHR(const VkImageMemoryRequirementsInfo2* pInfo, VkMemoryRequirements2* pMemoryRequirements) const {
	_table.vkGetImageMemoryRequirements2KHR(_device, pInfo, pMemoryRequirements);
}
inline void GetImageSparseMemoryRequirements2KHR(const VkImageSparseMemoryRequirementsInfo2* pInfo, uint32_t* pSparseMemoryRequirementCount, VkSparseImageMemoryRequirements2* pSparseMemoryRequirements) const {
	_table.vkGetImageSparseMemoryRequirements2KHR(_device, pInfo, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
}
#endif /* defined(VK_KHR_get_memory_requirements2) */
#if defined(VK_KHR_maintenance1)
inline void TrimCommandPoolKHR(VkCommandPool commandPool, VkCommandPoolTrimFlags flags) const {
	_table.vkTrimCommandPoolKHR(_device, commandPool, flags);
}
#endif /* defined(VK_KHR_maintenance1) */
#if defined(VK_KHR_maintenance3)
inline void GetDescriptorSetLayoutSupportKHR(const VkDescriptorSetLayoutCreateInfo* pCreateInfo, VkDescriptorSetLayoutSupport* pSupport) const {
	_table.vkGetDescriptorSetLayoutSupportKHR(_device, pCreateInfo, pSupport);
}
#endif /* defined(VK_KHR_maintenance3) */
#if defined(VK_KHR_maintenance4)
inline void GetDeviceBufferMemoryRequirementsKHR(const VkDeviceBufferMemoryRequirements* pInfo, VkMemoryRequirements2* pMemoryRequirements) const {
	_table.vkGetDeviceBufferMemoryRequirementsKHR(_device, pInfo, pMemoryRequirements);
}
inline void GetDeviceImageMemoryRequirementsKHR(const VkDeviceImageMemoryRequirements* pInfo, VkMemoryRequirements2* pMemoryRequirements) const {
	_table.vkGetDeviceImageMemoryRequirementsKHR(_device, pInfo, pMemoryRequirements);
}
inline void GetDeviceImageSparseMemoryRequirementsKHR(const VkDeviceImageMemoryRequirements* pInfo, uint32_t* pSparseMemoryRequirementCount, VkSparseImageMemoryRequirements2* pSparseMemoryRequirements) const {
	_table.vkGetDeviceImageSparseMemoryRequirementsKHR(_device, pInfo, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
}
#endif /* defined(VK_KHR_maintenance4) */
#if defined(VK_KHR_performance_query)
inline VkResult AcquireProfilingLockKHR(const VkAcquireProfilingLockInfoKHR* pInfo) const {
	return _table.vkAcquireProfilingLockKHR(_device, pInfo);
}
inline void ReleaseProfilingLockKHR() const {
	_table.vkReleaseProfilingLockKHR(_device);
}
#endif /* defined(VK_KHR_performance_query) */
#if defined(VK_KHR_pipeline_executable_properties)
inline VkResult GetPipelineExecutableInternalRepresentationsKHR(const VkPipelineExecutableInfoKHR*  pExecutableInfo, uint32_t* pInternalRepresentationCount, VkPipelineExecutableInternalRepresentationKHR* pInternalRepresentations) const {
	return _table.vkGetPipelineExecutableInternalRepresentationsKHR(_device, pExecutableInfo, pInternalRepresentationCount, pInternalRepresentations);
}
inline VkResult GetPipelineExecutablePropertiesKHR(const VkPipelineInfoKHR*        pPipelineInfo, uint32_t* pExecutableCount, VkPipelineExecutablePropertiesKHR* pProperties) const {
	return _table.vkGetPipelineExecutablePropertiesKHR(_device, pPipelineInfo, pExecutableCount, pProperties);
}
inline VkResult GetPipelineExecutableStatisticsKHR(const VkPipelineExecutableInfoKHR*  pExecutableInfo, uint32_t* pStatisticCount, VkPipelineExecutableStatisticKHR* pStatistics) const {
	return _table.vkGetPipelineExecutableStatisticsKHR(_device, pExecutableInfo, pStatisticCount, pStatistics);
}
#endif /* defined(VK_KHR_pipeline_executable_properties) */
#if defined(VK_KHR_present_wait)
inline VkResult WaitForPresentKHR(VkSwapchainKHR swapchain, uint64_t presentId, uint64_t timeout) const {
	return _table.vkWaitForPresentKHR(_device, swapchain, presentId, timeout);
}
#endif /* defined(VK_KHR_present_wait) */
#if defined(VK_KHR_push_descriptor)
inline void CmdPushDescriptorSetKHR(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout, uint32_t set, uint32_t descriptorWriteCount, const VkWriteDescriptorSet* pDescriptorWrites) const {
	_table.vkCmdPushDescriptorSetKHR(commandBuffer, pipelineBindPoint, layout, set, descriptorWriteCount, pDescriptorWrites);
}
#endif /* defined(VK_KHR_push_descriptor) */
#if defined(VK_KHR_ray_tracing_maintenance1) && defined(VK_KHR_ray_tracing_pipeline)
inline void CmdTraceRaysIndirect2KHR(VkCommandBuffer commandBuffer, VkDeviceAddress indirectDeviceAddress) const {
	_table.vkCmdTraceRaysIndirect2KHR(commandBuffer, indirectDeviceAddress);
}
#endif /* defined(VK_KHR_ray_tracing_maintenance1) && defined(VK_KHR_ray_tracing_pipeline) */
#if defined(VK_KHR_ray_tracing_pipeline)
inline void CmdSetRayTracingPipelineStackSizeKHR(VkCommandBuffer commandBuffer, uint32_t pipelineStackSize) const {
	_table.vkCmdSetRayTracingPipelineStackSizeKHR(commandBuffer, pipelineStackSize);
}
inline void CmdTraceRaysIndirectKHR(VkCommandBuffer commandBuffer, const VkStridedDeviceAddressRegionKHR* pRaygenShaderBindingTable, const VkStridedDeviceAddressRegionKHR* pMissShaderBindingTable, const VkStridedDeviceAddressRegionKHR* pHitShaderBindingTable, const VkStridedDeviceAddressRegionKHR* pCallableShaderBindingTable, VkDeviceAddress indirectDeviceAddress) const {
	_table.vkCmdTraceRaysIndirectKHR(commandBuffer, pRaygenShaderBindingTable, pMissShaderBindingTable, pHitShaderBindingTable, pCallableShaderBindingTable, indirectDeviceAddress);
}
inline void CmdTraceRaysKHR(VkCommandBuffer commandBuffer, const VkStridedDeviceAddressRegionKHR* pRaygenShaderBindingTable, const VkStridedDeviceAddressRegionKHR* pMissShaderBindingTable, const VkStridedDeviceAddressRegionKHR* pHitShaderBindingTable, const VkStridedDeviceAddressRegionKHR* pCallableShaderBindingTable, uint32_t width, uint32_t height, uint32_t depth) const {
	_table.vkCmdTraceRaysKHR(commandBuffer, pRaygenShaderBindingTable, pMissShaderBindingTable, pHitShaderBindingTable, pCallableShaderBindingTable, width, height, depth);
}
inline VkResult Create(VkDeferredOperationKHR deferredOperation, VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkRayTracingPipelineCreateInfoKHR* pCreateInfos, VkPipeline* pPipelines) const {
	return _table.vkCreateRayTracingPipelinesKHR(_device, deferredOperation, pipelineCache, createInfoCount, pCreateInfos, _allocator, pPipelines);
}
inline VkResult CreateRayTracingPipelinesKHR(VkDeferredOperationKHR deferredOperation, VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkRayTracingPipelineCreateInfoKHR* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines) const {
	return _table.vkCreateRayTracingPipelinesKHR(_device, deferredOperation, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines);
}
inline VkResult Create(VkDeferredOperationKHR deferredOperation, VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkRayTracingPipelineCreateInfoKHR* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines) const {
	return _table.vkCreateRayTracingPipelinesKHR(_device, deferredOperation, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines);
}
inline VkResult CreateRayTracingPipelinesKHR(VkDeferredOperationKHR deferredOperation, VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkRayTracingPipelineCreateInfoKHR* pCreateInfos, VkPipeline* pPipelines) const {
	return _table.vkCreateRayTracingPipelinesKHR(_device, deferredOperation, pipelineCache, createInfoCount, pCreateInfos, _allocator, pPipelines);
}
inline VkResult GetRayTracingCaptureReplayShaderGroupHandlesKHR(VkPipeline pipeline, uint32_t firstGroup, uint32_t groupCount, size_t dataSize, void* pData) const {
	return _table.vkGetRayTracingCaptureReplayShaderGroupHandlesKHR(_device, pipeline, firstGroup, groupCount, dataSize, pData);
}
inline VkResult GetRayTracingShaderGroupHandlesKHR(VkPipeline pipeline, uint32_t firstGroup, uint32_t groupCount, size_t dataSize, void* pData) const {
	return _table.vkGetRayTracingShaderGroupHandlesKHR(_device, pipeline, firstGroup, groupCount, dataSize, pData);
}
inline VkDeviceSize GetRayTracingShaderGroupStackSizeKHR(VkPipeline pipeline, uint32_t group, VkShaderGroupShaderKHR groupShader) const {
	return _table.vkGetRayTracingShaderGroupStackSizeKHR(_device, pipeline, group, groupShader);
}
#endif /* defined(VK_KHR_ray_tracing_pipeline) */
#if defined(VK_KHR_sampler_ycbcr_conversion)
#if !defined(VK_VERSION_1_1)
inline VkResult Create(const VkSamplerYcbcrConversionCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSamplerYcbcrConversion* pYcbcrConversion) const {
	return _table.vkCreateSamplerYcbcrConversionKHR(_device, pCreateInfo, pAllocator, pYcbcrConversion);
}
#endif
#if !defined(VK_VERSION_1_1)
inline VkResult CreateSamplerYcbcrConversionKHR(const VkSamplerYcbcrConversionCreateInfo* pCreateInfo, VkSamplerYcbcrConversion* pYcbcrConversion) const {
	return _table.vkCreateSamplerYcbcrConversionKHR(_device, pCreateInfo, _allocator, pYcbcrConversion);
}
#endif
#if !defined(VK_VERSION_1_1)
inline VkResult CreateSamplerYcbcrConversionKHR(const VkSamplerYcbcrConversionCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSamplerYcbcrConversion* pYcbcrConversion) const {
	return _table.vkCreateSamplerYcbcrConversionKHR(_device, pCreateInfo, pAllocator, pYcbcrConversion);
}
#endif
#if !defined(VK_VERSION_1_1)
inline VkResult Create(const VkSamplerYcbcrConversionCreateInfo* pCreateInfo, VkSamplerYcbcrConversion* pYcbcrConversion) const {
	return _table.vkCreateSamplerYcbcrConversionKHR(_device, pCreateInfo, _allocator, pYcbcrConversion);
}
#endif
#if !defined(VK_VERSION_1_1)
inline void Destroy(VkSamplerYcbcrConversion ycbcrConversion, const VkAllocationCallbacks* pAllocator) const {
	_table.vkDestroySamplerYcbcrConversionKHR(_device, ycbcrConversion, pAllocator);
}
#endif
#if !defined(VK_VERSION_1_1)
inline void DestroySamplerYcbcrConversionKHR(VkSamplerYcbcrConversion ycbcrConversion, const VkAllocationCallbacks* pAllocator) const {
	_table.vkDestroySamplerYcbcrConversionKHR(_device, ycbcrConversion, pAllocator);
}
#endif
#if !defined(VK_VERSION_1_1)
inline void DestroySamplerYcbcrConversionKHR(VkSamplerYcbcrConversion ycbcrConversion) const {
	_table.vkDestroySamplerYcbcrConversionKHR(_device, ycbcrConversion, _allocator);
}
#endif
#if !defined(VK_VERSION_1_1)
inline void Destroy(VkSamplerYcbcrConversion ycbcrConversion) const {
	_table.vkDestroySamplerYcbcrConversionKHR(_device, ycbcrConversion, _allocator);
}
#endif
#endif /* defined(VK_KHR_sampler_ycbcr_conversion) */
#if defined(VK_KHR_shared_presentable_image)
inline VkResult GetSwapchainStatusKHR(VkSwapchainKHR swapchain) const {
	return _table.vkGetSwapchainStatusKHR(_device, swapchain);
}
#endif /* defined(VK_KHR_shared_presentable_image) */
#if defined(VK_KHR_swapchain)
inline VkResult AcquireNextImageKHR(VkSwapchainKHR swapchain, uint64_t timeout, VkSemaphore semaphore, VkFence fence, uint32_t* pImageIndex) const {
	return _table.vkAcquireNextImageKHR(_device, swapchain, timeout, semaphore, fence, pImageIndex);
}
inline VkResult CreateSwapchainKHR(const VkSwapchainCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchain) const {
	return _table.vkCreateSwapchainKHR(_device, pCreateInfo, pAllocator, pSwapchain);
}
inline VkResult Create(const VkSwapchainCreateInfoKHR* pCreateInfo, VkSwapchainKHR* pSwapchain) const {
	return _table.vkCreateSwapchainKHR(_device, pCreateInfo, _allocator, pSwapchain);
}
inline VkResult Create(const VkSwapchainCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchain) const {
	return _table.vkCreateSwapchainKHR(_device, pCreateInfo, pAllocator, pSwapchain);
}
inline VkResult CreateSwapchainKHR(const VkSwapchainCreateInfoKHR* pCreateInfo, VkSwapchainKHR* pSwapchain) const {
	return _table.vkCreateSwapchainKHR(_device, pCreateInfo, _allocator, pSwapchain);
}
inline void DestroySwapchainKHR(VkSwapchainKHR swapchain) const {
	_table.vkDestroySwapchainKHR(_device, swapchain, _allocator);
}
inline void Destroy(VkSwapchainKHR swapchain, const VkAllocationCallbacks* pAllocator) const {
	_table.vkDestroySwapchainKHR(_device, swapchain, pAllocator);
}
inline void Destroy(VkSwapchainKHR swapchain) const {
	_table.vkDestroySwapchainKHR(_device, swapchain, _allocator);
}
inline void DestroySwapchainKHR(VkSwapchainKHR swapchain, const VkAllocationCallbacks* pAllocator) const {
	_table.vkDestroySwapchainKHR(_device, swapchain, pAllocator);
}
inline VkResult GetSwapchainImagesKHR(VkSwapchainKHR swapchain, uint32_t* pSwapchainImageCount, VkImage* pSwapchainImages) const {
	return _table.vkGetSwapchainImagesKHR(_device, swapchain, pSwapchainImageCount, pSwapchainImages);
}
inline VkResult QueuePresentKHR(VkQueue queue, const VkPresentInfoKHR* pPresentInfo) const {
	return _table.vkQueuePresentKHR(queue, pPresentInfo);
}
#endif /* defined(VK_KHR_swapchain) */
#if defined(VK_KHR_synchronization2)
inline void CmdPipelineBarrier2KHR(VkCommandBuffer                   commandBuffer, const VkDependencyInfo*                             pDependencyInfo) const {
	_table.vkCmdPipelineBarrier2KHR(commandBuffer, pDependencyInfo);
}
inline void CmdResetEvent2KHR(VkCommandBuffer                   commandBuffer, VkEvent                                             event, VkPipelineStageFlags2               stageMask) const {
	_table.vkCmdResetEvent2KHR(commandBuffer, event, stageMask);
}
inline void CmdSetEvent2KHR(VkCommandBuffer                   commandBuffer, VkEvent                                             event, const VkDependencyInfo*                             pDependencyInfo) const {
	_table.vkCmdSetEvent2KHR(commandBuffer, event, pDependencyInfo);
}
inline void CmdWaitEvents2KHR(VkCommandBuffer                   commandBuffer, uint32_t                                            eventCount, const VkEvent*                     pEvents, const VkDependencyInfo*            pDependencyInfos) const {
	_table.vkCmdWaitEvents2KHR(commandBuffer, eventCount, pEvents, pDependencyInfos);
}
inline void CmdWriteTimestamp2KHR(VkCommandBuffer                   commandBuffer, VkPipelineStageFlags2               stage, VkQueryPool                                         queryPool, uint32_t                                            query) const {
	_table.vkCmdWriteTimestamp2KHR(commandBuffer, stage, queryPool, query);
}
inline VkResult QueueSubmit2KHR(VkQueue                           queue, uint32_t                            submitCount, const VkSubmitInfo2*              pSubmits, VkFence           fence) const {
	return _table.vkQueueSubmit2KHR(queue, submitCount, pSubmits, fence);
}
#endif /* defined(VK_KHR_synchronization2) */
#if defined(VK_KHR_synchronization2) && defined(VK_AMD_buffer_marker)
inline void CmdWriteBufferMarker2AMD(VkCommandBuffer                   commandBuffer, VkPipelineStageFlags2               stage, VkBuffer                                            dstBuffer, VkDeviceSize                                        dstOffset, uint32_t                                            marker) const {
	_table.vkCmdWriteBufferMarker2AMD(commandBuffer, stage, dstBuffer, dstOffset, marker);
}
#endif /* defined(VK_KHR_synchronization2) && defined(VK_AMD_buffer_marker) */
#if defined(VK_KHR_synchronization2) && defined(VK_NV_device_diagnostic_checkpoints)
inline void GetQueueCheckpointData2NV(VkQueue queue, uint32_t* pCheckpointDataCount, VkCheckpointData2NV* pCheckpointData) const {
	_table.vkGetQueueCheckpointData2NV(queue, pCheckpointDataCount, pCheckpointData);
}
#endif /* defined(VK_KHR_synchronization2) && defined(VK_NV_device_diagnostic_checkpoints) */
#if defined(VK_KHR_timeline_semaphore)
inline VkResult GetSemaphoreCounterValueKHR(VkSemaphore semaphore, uint64_t* pValue) const {
	return _table.vkGetSemaphoreCounterValueKHR(_device, semaphore, pValue);
}
inline VkResult SignalSemaphoreKHR(const VkSemaphoreSignalInfo* pSignalInfo) const {
	return _table.vkSignalSemaphoreKHR(_device, pSignalInfo);
}
inline VkResult WaitSemaphoresKHR(const VkSemaphoreWaitInfo* pWaitInfo, uint64_t timeout) const {
	return _table.vkWaitSemaphoresKHR(_device, pWaitInfo, timeout);
}
#endif /* defined(VK_KHR_timeline_semaphore) */
#if defined(VK_KHR_video_decode_queue)
inline void CmdDecodeVideoKHR(VkCommandBuffer commandBuffer, const VkVideoDecodeInfoKHR* pFrameInfo) const {
	_table.vkCmdDecodeVideoKHR(commandBuffer, pFrameInfo);
}
#endif /* defined(VK_KHR_video_decode_queue) */
#if defined(VK_KHR_video_encode_queue)
inline void CmdEncodeVideoKHR(VkCommandBuffer commandBuffer, const VkVideoEncodeInfoKHR* pEncodeInfo) const {
	_table.vkCmdEncodeVideoKHR(commandBuffer, pEncodeInfo);
}
#endif /* defined(VK_KHR_video_encode_queue) */
#if defined(VK_KHR_video_queue)
inline VkResult BindVideoSessionMemoryKHR(VkVideoSessionKHR videoSession, uint32_t videoSessionBindMemoryCount, const VkVideoBindMemoryKHR* pVideoSessionBindMemories) const {
	return _table.vkBindVideoSessionMemoryKHR(_device, videoSession, videoSessionBindMemoryCount, pVideoSessionBindMemories);
}
inline void CmdBeginVideoCodingKHR(VkCommandBuffer commandBuffer, const VkVideoBeginCodingInfoKHR* pBeginInfo) const {
	_table.vkCmdBeginVideoCodingKHR(commandBuffer, pBeginInfo);
}
inline void CmdControlVideoCodingKHR(VkCommandBuffer commandBuffer, const VkVideoCodingControlInfoKHR* pCodingControlInfo) const {
	_table.vkCmdControlVideoCodingKHR(commandBuffer, pCodingControlInfo);
}
inline void CmdEndVideoCodingKHR(VkCommandBuffer commandBuffer, const VkVideoEndCodingInfoKHR* pEndCodingInfo) const {
	_table.vkCmdEndVideoCodingKHR(commandBuffer, pEndCodingInfo);
}
inline VkResult CreateVideoSessionKHR(const VkVideoSessionCreateInfoKHR* pCreateInfo, VkVideoSessionKHR* pVideoSession) const {
	return _table.vkCreateVideoSessionKHR(_device, pCreateInfo, _allocator, pVideoSession);
}
inline VkResult Create(const VkVideoSessionCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkVideoSessionKHR* pVideoSession) const {
	return _table.vkCreateVideoSessionKHR(_device, pCreateInfo, pAllocator, pVideoSession);
}
inline VkResult Create(const VkVideoSessionCreateInfoKHR* pCreateInfo, VkVideoSessionKHR* pVideoSession) const {
	return _table.vkCreateVideoSessionKHR(_device, pCreateInfo, _allocator, pVideoSession);
}
inline VkResult CreateVideoSessionKHR(const VkVideoSessionCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkVideoSessionKHR* pVideoSession) const {
	return _table.vkCreateVideoSessionKHR(_device, pCreateInfo, pAllocator, pVideoSession);
}
inline VkResult Create(const VkVideoSessionParametersCreateInfoKHR* pCreateInfo, VkVideoSessionParametersKHR* pVideoSessionParameters) const {
	return _table.vkCreateVideoSessionParametersKHR(_device, pCreateInfo, _allocator, pVideoSessionParameters);
}
inline VkResult Create(const VkVideoSessionParametersCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkVideoSessionParametersKHR* pVideoSessionParameters) const {
	return _table.vkCreateVideoSessionParametersKHR(_device, pCreateInfo, pAllocator, pVideoSessionParameters);
}
inline VkResult CreateVideoSessionParametersKHR(const VkVideoSessionParametersCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkVideoSessionParametersKHR* pVideoSessionParameters) const {
	return _table.vkCreateVideoSessionParametersKHR(_device, pCreateInfo, pAllocator, pVideoSessionParameters);
}
inline VkResult CreateVideoSessionParametersKHR(const VkVideoSessionParametersCreateInfoKHR* pCreateInfo, VkVideoSessionParametersKHR* pVideoSessionParameters) const {
	return _table.vkCreateVideoSessionParametersKHR(_device, pCreateInfo, _allocator, pVideoSessionParameters);
}
inline void DestroyVideoSessionKHR(VkVideoSessionKHR videoSession, const VkAllocationCallbacks* pAllocator) const {
	_table.vkDestroyVideoSessionKHR(_device, videoSession, pAllocator);
}
inline void Destroy(VkVideoSessionKHR videoSession) const {
	_table.vkDestroyVideoSessionKHR(_device, videoSession, _allocator);
}
inline void Destroy(VkVideoSessionKHR videoSession, const VkAllocationCallbacks* pAllocator) const {
	_table.vkDestroyVideoSessionKHR(_device, videoSession, pAllocator);
}
inline void DestroyVideoSessionKHR(VkVideoSessionKHR videoSession) const {
	_table.vkDestroyVideoSessionKHR(_device, videoSession, _allocator);
}
inline void Destroy(VkVideoSessionParametersKHR videoSessionParameters, const VkAllocationCallbacks* pAllocator) const {
	_table.vkDestroyVideoSessionParametersKHR(_device, videoSessionParameters, pAllocator);
}
inline void Destroy(VkVideoSessionParametersKHR videoSessionParameters) const {
	_table.vkDestroyVideoSessionParametersKHR(_device, videoSessionParameters, _allocator);
}
inline void DestroyVideoSessionParametersKHR(VkVideoSessionParametersKHR videoSessionParameters) const {
	_table.vkDestroyVideoSessionParametersKHR(_device, videoSessionParameters, _allocator);
}
inline void DestroyVideoSessionParametersKHR(VkVideoSessionParametersKHR videoSessionParameters, const VkAllocationCallbacks* pAllocator) const {
	_table.vkDestroyVideoSessionParametersKHR(_device, videoSessionParameters, pAllocator);
}
inline VkResult GetVideoSessionMemoryRequirementsKHR(VkVideoSessionKHR videoSession, uint32_t* pVideoSessionMemoryRequirementsCount, VkVideoGetMemoryPropertiesKHR* pVideoSessionMemoryRequirements) const {
	return _table.vkGetVideoSessionMemoryRequirementsKHR(_device, videoSession, pVideoSessionMemoryRequirementsCount, pVideoSessionMemoryRequirements);
}
inline VkResult UpdateVideoSessionParametersKHR(VkVideoSessionParametersKHR videoSessionParameters, const VkVideoSessionParametersUpdateInfoKHR* pUpdateInfo) const {
	return _table.vkUpdateVideoSessionParametersKHR(_device, videoSessionParameters, pUpdateInfo);
}
#endif /* defined(VK_KHR_video_queue) */
#if defined(VK_NVX_binary_import)
inline void CmdCuLaunchKernelNVX(VkCommandBuffer commandBuffer, const VkCuLaunchInfoNVX* pLaunchInfo) const {
	_table.vkCmdCuLaunchKernelNVX(commandBuffer, pLaunchInfo);
}
inline VkResult CreateCuFunctionNVX(const VkCuFunctionCreateInfoNVX* pCreateInfo, VkCuFunctionNVX* pFunction) const {
	return _table.vkCreateCuFunctionNVX(_device, pCreateInfo, _allocator, pFunction);
}
inline VkResult CreateCuFunctionNVX(const VkCuFunctionCreateInfoNVX* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkCuFunctionNVX* pFunction) const {
	return _table.vkCreateCuFunctionNVX(_device, pCreateInfo, pAllocator, pFunction);
}
inline VkResult Create(const VkCuFunctionCreateInfoNVX* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkCuFunctionNVX* pFunction) const {
	return _table.vkCreateCuFunctionNVX(_device, pCreateInfo, pAllocator, pFunction);
}
inline VkResult Create(const VkCuFunctionCreateInfoNVX* pCreateInfo, VkCuFunctionNVX* pFunction) const {
	return _table.vkCreateCuFunctionNVX(_device, pCreateInfo, _allocator, pFunction);
}
inline VkResult Create(const VkCuModuleCreateInfoNVX* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkCuModuleNVX* pModule) const {
	return _table.vkCreateCuModuleNVX(_device, pCreateInfo, pAllocator, pModule);
}
inline VkResult CreateCuModuleNVX(const VkCuModuleCreateInfoNVX* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkCuModuleNVX* pModule) const {
	return _table.vkCreateCuModuleNVX(_device, pCreateInfo, pAllocator, pModule);
}
inline VkResult CreateCuModuleNVX(const VkCuModuleCreateInfoNVX* pCreateInfo, VkCuModuleNVX* pModule) const {
	return _table.vkCreateCuModuleNVX(_device, pCreateInfo, _allocator, pModule);
}
inline VkResult Create(const VkCuModuleCreateInfoNVX* pCreateInfo, VkCuModuleNVX* pModule) const {
	return _table.vkCreateCuModuleNVX(_device, pCreateInfo, _allocator, pModule);
}
inline void Destroy(VkCuFunctionNVX function, const VkAllocationCallbacks* pAllocator) const {
	_table.vkDestroyCuFunctionNVX(_device, function, pAllocator);
}
inline void DestroyCuFunctionNVX(VkCuFunctionNVX function, const VkAllocationCallbacks* pAllocator) const {
	_table.vkDestroyCuFunctionNVX(_device, function, pAllocator);
}
inline void Destroy(VkCuFunctionNVX function) const {
	_table.vkDestroyCuFunctionNVX(_device, function, _allocator);
}
inline void DestroyCuFunctionNVX(VkCuFunctionNVX function) const {
	_table.vkDestroyCuFunctionNVX(_device, function, _allocator);
}
inline void Destroy(VkCuModuleNVX module) const {
	_table.vkDestroyCuModuleNVX(_device, module, _allocator);
}
inline void DestroyCuModuleNVX(VkCuModuleNVX module) const {
	_table.vkDestroyCuModuleNVX(_device, module, _allocator);
}
inline void Destroy(VkCuModuleNVX module, const VkAllocationCallbacks* pAllocator) const {
	_table.vkDestroyCuModuleNVX(_device, module, pAllocator);
}
inline void DestroyCuModuleNVX(VkCuModuleNVX module, const VkAllocationCallbacks* pAllocator) const {
	_table.vkDestroyCuModuleNVX(_device, module, pAllocator);
}
#endif /* defined(VK_NVX_binary_import) */
#if defined(VK_NVX_image_view_handle)
inline VkResult GetImageViewAddressNVX(VkImageView imageView, VkImageViewAddressPropertiesNVX* pProperties) const {
	return _table.vkGetImageViewAddressNVX(_device, imageView, pProperties);
}
inline uint32_t GetImageViewHandleNVX(const VkImageViewHandleInfoNVX* pInfo) const {
	return _table.vkGetImageViewHandleNVX(_device, pInfo);
}
#endif /* defined(VK_NVX_image_view_handle) */
#if defined(VK_NV_clip_space_w_scaling)
inline void CmdSetViewportWScalingNV(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount, const VkViewportWScalingNV* pViewportWScalings) const {
	_table.vkCmdSetViewportWScalingNV(commandBuffer, firstViewport, viewportCount, pViewportWScalings);
}
#endif /* defined(VK_NV_clip_space_w_scaling) */
#if defined(VK_NV_device_diagnostic_checkpoints)
inline void CmdSetCheckpointNV(VkCommandBuffer commandBuffer, const void* pCheckpointMarker) const {
	_table.vkCmdSetCheckpointNV(commandBuffer, pCheckpointMarker);
}
inline void GetQueueCheckpointDataNV(VkQueue queue, uint32_t* pCheckpointDataCount, VkCheckpointDataNV* pCheckpointData) const {
	_table.vkGetQueueCheckpointDataNV(queue, pCheckpointDataCount, pCheckpointData);
}
#endif /* defined(VK_NV_device_diagnostic_checkpoints) */
#if defined(VK_NV_device_generated_commands)
inline void CmdBindPipelineShaderGroupNV(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipeline pipeline, uint32_t groupIndex) const {
	_table.vkCmdBindPipelineShaderGroupNV(commandBuffer, pipelineBindPoint, pipeline, groupIndex);
}
inline void CmdExecuteGeneratedCommandsNV(VkCommandBuffer commandBuffer, VkBool32 isPreprocessed, const VkGeneratedCommandsInfoNV* pGeneratedCommandsInfo) const {
	_table.vkCmdExecuteGeneratedCommandsNV(commandBuffer, isPreprocessed, pGeneratedCommandsInfo);
}
inline void CmdPreprocessGeneratedCommandsNV(VkCommandBuffer commandBuffer, const VkGeneratedCommandsInfoNV* pGeneratedCommandsInfo) const {
	_table.vkCmdPreprocessGeneratedCommandsNV(commandBuffer, pGeneratedCommandsInfo);
}
inline VkResult Create(const VkIndirectCommandsLayoutCreateInfoNV* pCreateInfo, VkIndirectCommandsLayoutNV* pIndirectCommandsLayout) const {
	return _table.vkCreateIndirectCommandsLayoutNV(_device, pCreateInfo, _allocator, pIndirectCommandsLayout);
}
inline VkResult Create(const VkIndirectCommandsLayoutCreateInfoNV* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkIndirectCommandsLayoutNV* pIndirectCommandsLayout) const {
	return _table.vkCreateIndirectCommandsLayoutNV(_device, pCreateInfo, pAllocator, pIndirectCommandsLayout);
}
inline VkResult CreateIndirectCommandsLayoutNV(const VkIndirectCommandsLayoutCreateInfoNV* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkIndirectCommandsLayoutNV* pIndirectCommandsLayout) const {
	return _table.vkCreateIndirectCommandsLayoutNV(_device, pCreateInfo, pAllocator, pIndirectCommandsLayout);
}
inline VkResult CreateIndirectCommandsLayoutNV(const VkIndirectCommandsLayoutCreateInfoNV* pCreateInfo, VkIndirectCommandsLayoutNV* pIndirectCommandsLayout) const {
	return _table.vkCreateIndirectCommandsLayoutNV(_device, pCreateInfo, _allocator, pIndirectCommandsLayout);
}
inline void DestroyIndirectCommandsLayoutNV(VkIndirectCommandsLayoutNV indirectCommandsLayout, const VkAllocationCallbacks* pAllocator) const {
	_table.vkDestroyIndirectCommandsLayoutNV(_device, indirectCommandsLayout, pAllocator);
}
inline void Destroy(VkIndirectCommandsLayoutNV indirectCommandsLayout, const VkAllocationCallbacks* pAllocator) const {
	_table.vkDestroyIndirectCommandsLayoutNV(_device, indirectCommandsLayout, pAllocator);
}
inline void DestroyIndirectCommandsLayoutNV(VkIndirectCommandsLayoutNV indirectCommandsLayout) const {
	_table.vkDestroyIndirectCommandsLayoutNV(_device, indirectCommandsLayout, _allocator);
}
inline void Destroy(VkIndirectCommandsLayoutNV indirectCommandsLayout) const {
	_table.vkDestroyIndirectCommandsLayoutNV(_device, indirectCommandsLayout, _allocator);
}
inline void GetGeneratedCommandsMemoryRequirementsNV(const VkGeneratedCommandsMemoryRequirementsInfoNV* pInfo, VkMemoryRequirements2* pMemoryRequirements) const {
	_table.vkGetGeneratedCommandsMemoryRequirementsNV(_device, pInfo, pMemoryRequirements);
}
#endif /* defined(VK_NV_device_generated_commands) */
#if defined(VK_NV_external_memory_rdma)
inline VkResult GetMemoryRemoteAddressNV(const VkMemoryGetRemoteAddressInfoNV* pMemoryGetRemoteAddressInfo, VkRemoteAddressNV* pAddress) const {
	return _table.vkGetMemoryRemoteAddressNV(_device, pMemoryGetRemoteAddressInfo, pAddress);
}
#endif /* defined(VK_NV_external_memory_rdma) */
#if defined(VK_NV_external_memory_win32)
inline VkResult GetMemoryWin32HandleNV(VkDeviceMemory memory, VkExternalMemoryHandleTypeFlagsNV handleType, HANDLE* pHandle) const {
	return _table.vkGetMemoryWin32HandleNV(_device, memory, handleType, pHandle);
}
#endif /* defined(VK_NV_external_memory_win32) */
#if defined(VK_NV_fragment_shading_rate_enums)
inline void CmdSetFragmentShadingRateEnumNV(VkCommandBuffer           commandBuffer, VkFragmentShadingRateNV                     shadingRate, const VkFragmentShadingRateCombinerOpKHR    combinerOps[2]) const {
	_table.vkCmdSetFragmentShadingRateEnumNV(commandBuffer, shadingRate, combinerOps);
}
#endif /* defined(VK_NV_fragment_shading_rate_enums) */
#if defined(VK_NV_mesh_shader)
inline void CmdDrawMeshTasksIndirectCountNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride) const {
	_table.vkCmdDrawMeshTasksIndirectCountNV(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
}
inline void CmdDrawMeshTasksIndirectNV(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount, uint32_t stride) const {
	_table.vkCmdDrawMeshTasksIndirectNV(commandBuffer, buffer, offset, drawCount, stride);
}
inline void CmdDrawMeshTasksNV(VkCommandBuffer commandBuffer, uint32_t taskCount, uint32_t firstTask) const {
	_table.vkCmdDrawMeshTasksNV(commandBuffer, taskCount, firstTask);
}
#endif /* defined(VK_NV_mesh_shader) */
#if defined(VK_NV_ray_tracing)
inline VkResult BindAccelerationStructureMemoryNV(uint32_t bindInfoCount, const VkBindAccelerationStructureMemoryInfoNV* pBindInfos) const {
	return _table.vkBindAccelerationStructureMemoryNV(_device, bindInfoCount, pBindInfos);
}
inline void CmdBuildAccelerationStructureNV(VkCommandBuffer commandBuffer, const VkAccelerationStructureInfoNV* pInfo, VkBuffer instanceData, VkDeviceSize instanceOffset, VkBool32 update, VkAccelerationStructureNV dst, VkAccelerationStructureNV src, VkBuffer scratch, VkDeviceSize scratchOffset) const {
	_table.vkCmdBuildAccelerationStructureNV(commandBuffer, pInfo, instanceData, instanceOffset, update, dst, src, scratch, scratchOffset);
}
inline void CmdCopyAccelerationStructureNV(VkCommandBuffer commandBuffer, VkAccelerationStructureNV dst, VkAccelerationStructureNV src, VkCopyAccelerationStructureModeKHR mode) const {
	_table.vkCmdCopyAccelerationStructureNV(commandBuffer, dst, src, mode);
}
inline void CmdTraceRaysNV(VkCommandBuffer commandBuffer, VkBuffer raygenShaderBindingTableBuffer, VkDeviceSize raygenShaderBindingOffset, VkBuffer missShaderBindingTableBuffer, VkDeviceSize missShaderBindingOffset, VkDeviceSize missShaderBindingStride, VkBuffer hitShaderBindingTableBuffer, VkDeviceSize hitShaderBindingOffset, VkDeviceSize hitShaderBindingStride, VkBuffer callableShaderBindingTableBuffer, VkDeviceSize callableShaderBindingOffset, VkDeviceSize callableShaderBindingStride, uint32_t width, uint32_t height, uint32_t depth) const {
	_table.vkCmdTraceRaysNV(commandBuffer, raygenShaderBindingTableBuffer, raygenShaderBindingOffset, missShaderBindingTableBuffer, missShaderBindingOffset, missShaderBindingStride, hitShaderBindingTableBuffer, hitShaderBindingOffset, hitShaderBindingStride, callableShaderBindingTableBuffer, callableShaderBindingOffset, callableShaderBindingStride, width, height, depth);
}
inline void CmdWriteAccelerationStructuresPropertiesNV(VkCommandBuffer commandBuffer, uint32_t accelerationStructureCount, const VkAccelerationStructureNV* pAccelerationStructures, VkQueryType queryType, VkQueryPool queryPool, uint32_t firstQuery) const {
	_table.vkCmdWriteAccelerationStructuresPropertiesNV(commandBuffer, accelerationStructureCount, pAccelerationStructures, queryType, queryPool, firstQuery);
}
inline VkResult CompileDeferredNV(VkPipeline pipeline, uint32_t shader) const {
	return _table.vkCompileDeferredNV(_device, pipeline, shader);
}
inline VkResult CreateAccelerationStructureNV(const VkAccelerationStructureCreateInfoNV* pCreateInfo, VkAccelerationStructureNV* pAccelerationStructure) const {
	return _table.vkCreateAccelerationStructureNV(_device, pCreateInfo, _allocator, pAccelerationStructure);
}
inline VkResult Create(const VkAccelerationStructureCreateInfoNV* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkAccelerationStructureNV* pAccelerationStructure) const {
	return _table.vkCreateAccelerationStructureNV(_device, pCreateInfo, pAllocator, pAccelerationStructure);
}
inline VkResult Create(const VkAccelerationStructureCreateInfoNV* pCreateInfo, VkAccelerationStructureNV* pAccelerationStructure) const {
	return _table.vkCreateAccelerationStructureNV(_device, pCreateInfo, _allocator, pAccelerationStructure);
}
inline VkResult CreateAccelerationStructureNV(const VkAccelerationStructureCreateInfoNV* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkAccelerationStructureNV* pAccelerationStructure) const {
	return _table.vkCreateAccelerationStructureNV(_device, pCreateInfo, pAllocator, pAccelerationStructure);
}
inline VkResult CreateRayTracingPipelinesNV(VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkRayTracingPipelineCreateInfoNV* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines) const {
	return _table.vkCreateRayTracingPipelinesNV(_device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines);
}
inline VkResult CreateRayTracingPipelinesNV(VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkRayTracingPipelineCreateInfoNV* pCreateInfos, VkPipeline* pPipelines) const {
	return _table.vkCreateRayTracingPipelinesNV(_device, pipelineCache, createInfoCount, pCreateInfos, _allocator, pPipelines);
}
inline VkResult Create(VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkRayTracingPipelineCreateInfoNV* pCreateInfos, VkPipeline* pPipelines) const {
	return _table.vkCreateRayTracingPipelinesNV(_device, pipelineCache, createInfoCount, pCreateInfos, _allocator, pPipelines);
}
inline VkResult Create(VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkRayTracingPipelineCreateInfoNV* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines) const {
	return _table.vkCreateRayTracingPipelinesNV(_device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines);
}
inline void DestroyAccelerationStructureNV(VkAccelerationStructureNV accelerationStructure) const {
	_table.vkDestroyAccelerationStructureNV(_device, accelerationStructure, _allocator);
}
inline void Destroy(VkAccelerationStructureNV accelerationStructure, const VkAllocationCallbacks* pAllocator) const {
	_table.vkDestroyAccelerationStructureNV(_device, accelerationStructure, pAllocator);
}
inline void DestroyAccelerationStructureNV(VkAccelerationStructureNV accelerationStructure, const VkAllocationCallbacks* pAllocator) const {
	_table.vkDestroyAccelerationStructureNV(_device, accelerationStructure, pAllocator);
}
inline void Destroy(VkAccelerationStructureNV accelerationStructure) const {
	_table.vkDestroyAccelerationStructureNV(_device, accelerationStructure, _allocator);
}
inline VkResult GetAccelerationStructureHandleNV(VkAccelerationStructureNV accelerationStructure, size_t dataSize, void* pData) const {
	return _table.vkGetAccelerationStructureHandleNV(_device, accelerationStructure, dataSize, pData);
}
inline void GetAccelerationStructureMemoryRequirementsNV(const VkAccelerationStructureMemoryRequirementsInfoNV* pInfo, VkMemoryRequirements2KHR* pMemoryRequirements) const {
	_table.vkGetAccelerationStructureMemoryRequirementsNV(_device, pInfo, pMemoryRequirements);
}
inline VkResult GetRayTracingShaderGroupHandlesNV(VkPipeline pipeline, uint32_t firstGroup, uint32_t groupCount, size_t dataSize, void* pData) const {
	return _table.vkGetRayTracingShaderGroupHandlesNV(_device, pipeline, firstGroup, groupCount, dataSize, pData);
}
#endif /* defined(VK_NV_ray_tracing) */
#if defined(VK_NV_scissor_exclusive)
inline void CmdSetExclusiveScissorNV(VkCommandBuffer commandBuffer, uint32_t firstExclusiveScissor, uint32_t exclusiveScissorCount, const VkRect2D* pExclusiveScissors) const {
	_table.vkCmdSetExclusiveScissorNV(commandBuffer, firstExclusiveScissor, exclusiveScissorCount, pExclusiveScissors);
}
#endif /* defined(VK_NV_scissor_exclusive) */
#if defined(VK_NV_shading_rate_image)
inline void CmdBindShadingRateImageNV(VkCommandBuffer commandBuffer, VkImageView imageView, VkImageLayout imageLayout) const {
	_table.vkCmdBindShadingRateImageNV(commandBuffer, imageView, imageLayout);
}
inline void CmdSetCoarseSampleOrderNV(VkCommandBuffer commandBuffer, VkCoarseSampleOrderTypeNV sampleOrderType, uint32_t customSampleOrderCount, const VkCoarseSampleOrderCustomNV* pCustomSampleOrders) const {
	_table.vkCmdSetCoarseSampleOrderNV(commandBuffer, sampleOrderType, customSampleOrderCount, pCustomSampleOrders);
}
inline void CmdSetViewportShadingRatePaletteNV(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount, const VkShadingRatePaletteNV* pShadingRatePalettes) const {
	_table.vkCmdSetViewportShadingRatePaletteNV(commandBuffer, firstViewport, viewportCount, pShadingRatePalettes);
}
#endif /* defined(VK_NV_shading_rate_image) */
#if defined(VK_VALVE_descriptor_set_host_mapping)
inline void GetDescriptorSetHostMappingVALVE(VkDescriptorSet descriptorSet, void** ppData) const {
	_table.vkGetDescriptorSetHostMappingVALVE(_device, descriptorSet, ppData);
}
inline void GetDescriptorSetLayoutHostMappingInfoVALVE(const VkDescriptorSetBindingReferenceVALVE* pBindingReference, VkDescriptorSetLayoutHostMappingInfoVALVE* pHostMapping) const {
	_table.vkGetDescriptorSetLayoutHostMappingInfoVALVE(_device, pBindingReference, pHostMapping);
}
#endif /* defined(VK_VALVE_descriptor_set_host_mapping) */
#if (defined(VK_EXT_full_screen_exclusive) && defined(VK_KHR_device_group)) || (defined(VK_EXT_full_screen_exclusive) && defined(VK_VERSION_1_1))
inline VkResult GetDeviceGroupSurfacePresentModes2EXT(const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo, VkDeviceGroupPresentModeFlagsKHR* pModes) const {
	return _table.vkGetDeviceGroupSurfacePresentModes2EXT(_device, pSurfaceInfo, pModes);
}
#endif /* (defined(VK_EXT_full_screen_exclusive) && defined(VK_KHR_device_group)) || (defined(VK_EXT_full_screen_exclusive) && defined(VK_VERSION_1_1)) */
#if (defined(VK_KHR_descriptor_update_template) && defined(VK_KHR_push_descriptor)) || (defined(VK_KHR_push_descriptor) && defined(VK_VERSION_1_1)) || (defined(VK_KHR_push_descriptor) && defined(VK_KHR_descriptor_update_template))
inline void CmdPushDescriptorSetWithTemplateKHR(VkCommandBuffer commandBuffer, VkDescriptorUpdateTemplate descriptorUpdateTemplate, VkPipelineLayout layout, uint32_t set, const void* pData) const {
	_table.vkCmdPushDescriptorSetWithTemplateKHR(commandBuffer, descriptorUpdateTemplate, layout, set, pData);
}
#endif /* (defined(VK_KHR_descriptor_update_template) && defined(VK_KHR_push_descriptor)) || (defined(VK_KHR_push_descriptor) && defined(VK_VERSION_1_1)) || (defined(VK_KHR_push_descriptor) && defined(VK_KHR_descriptor_update_template)) */
#if (defined(VK_KHR_device_group) && defined(VK_KHR_surface)) || (defined(VK_KHR_swapchain) && defined(VK_VERSION_1_1))
inline VkResult GetDeviceGroupPresentCapabilitiesKHR(VkDeviceGroupPresentCapabilitiesKHR* pDeviceGroupPresentCapabilities) const {
	return _table.vkGetDeviceGroupPresentCapabilitiesKHR(_device, pDeviceGroupPresentCapabilities);
}
inline VkResult GetDeviceGroupSurfacePresentModesKHR(VkSurfaceKHR surface, VkDeviceGroupPresentModeFlagsKHR* pModes) const {
	return _table.vkGetDeviceGroupSurfacePresentModesKHR(_device, surface, pModes);
}
#endif /* (defined(VK_KHR_device_group) && defined(VK_KHR_surface)) || (defined(VK_KHR_swapchain) && defined(VK_VERSION_1_1)) */
#if (defined(VK_KHR_device_group) && defined(VK_KHR_swapchain)) || (defined(VK_KHR_swapchain) && defined(VK_VERSION_1_1))
inline VkResult AcquireNextImage2KHR(const VkAcquireNextImageInfoKHR* pAcquireInfo, uint32_t* pImageIndex) const {
	return _table.vkAcquireNextImage2KHR(_device, pAcquireInfo, pImageIndex);
}
#endif /* (defined(VK_KHR_device_group) && defined(VK_KHR_swapchain)) || (defined(VK_KHR_swapchain) && defined(VK_VERSION_1_1)) */
  /* VOLK_GENERATE_DEVICE_METHOD_HPP */
};

class Instance {
protected:
  VkInstance _instance;
  const VkAllocationCallbacks *_allocator;
  VolkInstanceTable _table;

public:
  Instance(VkInstance instance,
           const VkAllocationCallbacks *allocator = nullptr)
      : _instance(instance), _allocator(allocator) {
    volkLoadInstanceTable(&_table, instance);
  }

  inline VkInstance instance() const {
    return _instance;
  }

  inline const VolkInstanceTable &table() const {
    return _table;
  }

  inline const VkAllocationCallbacks *allocator() const {
    return _allocator;
  }

  /* VOLK_GENERATE_INSTANCE_METHOD_HPP */
#if defined(VK_VERSION_1_0)
inline VkResult CreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDevice* pDevice) const {
	return _table.vkCreateDevice(physicalDevice, pCreateInfo, pAllocator, pDevice);
}
inline VkResult Create(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDevice* pDevice) const {
	return _table.vkCreateDevice(physicalDevice, pCreateInfo, pAllocator, pDevice);
}
inline VkResult CreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo* pCreateInfo, VkDevice* pDevice) const {
	return _table.vkCreateDevice(physicalDevice, pCreateInfo, _allocator, pDevice);
}
inline VkResult Create(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo* pCreateInfo, VkDevice* pDevice) const {
	return _table.vkCreateDevice(physicalDevice, pCreateInfo, _allocator, pDevice);
}
inline void Destroy() const {
	_table.vkDestroyInstance(_instance, _allocator);
}
inline void DestroyInstance() const {
	_table.vkDestroyInstance(_instance, _allocator);
}
inline void Destroy(const VkAllocationCallbacks* pAllocator) const {
	_table.vkDestroyInstance(_instance, pAllocator);
}
inline void DestroyInstance(const VkAllocationCallbacks* pAllocator) const {
	_table.vkDestroyInstance(_instance, pAllocator);
}
inline VkResult EnumerateDeviceExtensionProperties(VkPhysicalDevice physicalDevice, const char* pLayerName, uint32_t* pPropertyCount, VkExtensionProperties* pProperties) const {
	return _table.vkEnumerateDeviceExtensionProperties(physicalDevice, pLayerName, pPropertyCount, pProperties);
}
inline VkResult EnumerateDeviceLayerProperties(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount, VkLayerProperties* pProperties) const {
	return _table.vkEnumerateDeviceLayerProperties(physicalDevice, pPropertyCount, pProperties);
}
inline VkResult EnumeratePhysicalDevices(uint32_t* pPhysicalDeviceCount, VkPhysicalDevice* pPhysicalDevices) const {
	return _table.vkEnumeratePhysicalDevices(_instance, pPhysicalDeviceCount, pPhysicalDevices);
}
inline PFN_vkVoidFunction GetDeviceProcAddr(VkDevice device, const char* pName) const {
	return _table.vkGetDeviceProcAddr(device, pName);
}
inline void GetPhysicalDeviceFeatures(VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures* pFeatures) const {
	_table.vkGetPhysicalDeviceFeatures(physicalDevice, pFeatures);
}
inline void GetPhysicalDeviceFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format, VkFormatProperties* pFormatProperties) const {
	_table.vkGetPhysicalDeviceFormatProperties(physicalDevice, format, pFormatProperties);
}
inline VkResult GetPhysicalDeviceImageFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format, VkImageType type, VkImageTiling tiling, VkImageUsageFlags usage, VkImageCreateFlags flags, VkImageFormatProperties* pImageFormatProperties) const {
	return _table.vkGetPhysicalDeviceImageFormatProperties(physicalDevice, format, type, tiling, usage, flags, pImageFormatProperties);
}
inline void GetPhysicalDeviceMemoryProperties(VkPhysicalDevice physicalDevice, VkPhysicalDeviceMemoryProperties* pMemoryProperties) const {
	_table.vkGetPhysicalDeviceMemoryProperties(physicalDevice, pMemoryProperties);
}
inline void GetPhysicalDeviceProperties(VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties* pProperties) const {
	_table.vkGetPhysicalDeviceProperties(physicalDevice, pProperties);
}
inline void GetPhysicalDeviceQueueFamilyProperties(VkPhysicalDevice physicalDevice, uint32_t* pQueueFamilyPropertyCount, VkQueueFamilyProperties* pQueueFamilyProperties) const {
	_table.vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties);
}
inline void GetPhysicalDeviceSparseImageFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format, VkImageType type, VkSampleCountFlagBits samples, VkImageUsageFlags usage, VkImageTiling tiling, uint32_t* pPropertyCount, VkSparseImageFormatProperties* pProperties) const {
	_table.vkGetPhysicalDeviceSparseImageFormatProperties(physicalDevice, format, type, samples, usage, tiling, pPropertyCount, pProperties);
}
#endif /* defined(VK_VERSION_1_0) */
#if defined(VK_VERSION_1_1)
inline VkResult EnumeratePhysicalDeviceGroups(uint32_t* pPhysicalDeviceGroupCount, VkPhysicalDeviceGroupProperties* pPhysicalDeviceGroupProperties) const {
	return _table.vkEnumeratePhysicalDeviceGroups(_instance, pPhysicalDeviceGroupCount, pPhysicalDeviceGroupProperties);
}
inline void GetPhysicalDeviceExternalBufferProperties(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalBufferInfo* pExternalBufferInfo, VkExternalBufferProperties* pExternalBufferProperties) const {
	_table.vkGetPhysicalDeviceExternalBufferProperties(physicalDevice, pExternalBufferInfo, pExternalBufferProperties);
}
inline void GetPhysicalDeviceExternalFenceProperties(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalFenceInfo* pExternalFenceInfo, VkExternalFenceProperties* pExternalFenceProperties) const {
	_table.vkGetPhysicalDeviceExternalFenceProperties(physicalDevice, pExternalFenceInfo, pExternalFenceProperties);
}
inline void GetPhysicalDeviceExternalSemaphoreProperties(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalSemaphoreInfo* pExternalSemaphoreInfo, VkExternalSemaphoreProperties* pExternalSemaphoreProperties) const {
	_table.vkGetPhysicalDeviceExternalSemaphoreProperties(physicalDevice, pExternalSemaphoreInfo, pExternalSemaphoreProperties);
}
inline void GetPhysicalDeviceFeatures2(VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures2* pFeatures) const {
	_table.vkGetPhysicalDeviceFeatures2(physicalDevice, pFeatures);
}
inline void GetPhysicalDeviceFormatProperties2(VkPhysicalDevice physicalDevice, VkFormat format, VkFormatProperties2* pFormatProperties) const {
	_table.vkGetPhysicalDeviceFormatProperties2(physicalDevice, format, pFormatProperties);
}
inline VkResult GetPhysicalDeviceImageFormatProperties2(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceImageFormatInfo2* pImageFormatInfo, VkImageFormatProperties2* pImageFormatProperties) const {
	return _table.vkGetPhysicalDeviceImageFormatProperties2(physicalDevice, pImageFormatInfo, pImageFormatProperties);
}
inline void GetPhysicalDeviceMemoryProperties2(VkPhysicalDevice physicalDevice, VkPhysicalDeviceMemoryProperties2* pMemoryProperties) const {
	_table.vkGetPhysicalDeviceMemoryProperties2(physicalDevice, pMemoryProperties);
}
inline void GetPhysicalDeviceProperties2(VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties2* pProperties) const {
	_table.vkGetPhysicalDeviceProperties2(physicalDevice, pProperties);
}
inline void GetPhysicalDeviceQueueFamilyProperties2(VkPhysicalDevice physicalDevice, uint32_t* pQueueFamilyPropertyCount, VkQueueFamilyProperties2* pQueueFamilyProperties) const {
	_table.vkGetPhysicalDeviceQueueFamilyProperties2(physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties);
}
inline void GetPhysicalDeviceSparseImageFormatProperties2(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceSparseImageFormatInfo2* pFormatInfo, uint32_t* pPropertyCount, VkSparseImageFormatProperties2* pProperties) const {
	_table.vkGetPhysicalDeviceSparseImageFormatProperties2(physicalDevice, pFormatInfo, pPropertyCount, pProperties);
}
#endif /* defined(VK_VERSION_1_1) */
#if defined(VK_VERSION_1_3)
inline VkResult GetPhysicalDeviceToolProperties(VkPhysicalDevice physicalDevice, uint32_t* pToolCount, VkPhysicalDeviceToolProperties* pToolProperties) const {
	return _table.vkGetPhysicalDeviceToolProperties(physicalDevice, pToolCount, pToolProperties);
}
#endif /* defined(VK_VERSION_1_3) */
#if defined(VK_EXT_acquire_drm_display)
inline VkResult AcquireDrmDisplayEXT(VkPhysicalDevice physicalDevice, int32_t drmFd, VkDisplayKHR display) const {
	return _table.vkAcquireDrmDisplayEXT(physicalDevice, drmFd, display);
}
inline VkResult GetDrmDisplayEXT(VkPhysicalDevice physicalDevice, int32_t drmFd, uint32_t connectorId, VkDisplayKHR* display) const {
	return _table.vkGetDrmDisplayEXT(physicalDevice, drmFd, connectorId, display);
}
#endif /* defined(VK_EXT_acquire_drm_display) */
#if defined(VK_EXT_acquire_xlib_display)
inline VkResult AcquireXlibDisplayEXT(VkPhysicalDevice physicalDevice, Display* dpy, VkDisplayKHR display) const {
	return _table.vkAcquireXlibDisplayEXT(physicalDevice, dpy, display);
}
inline VkResult GetRandROutputDisplayEXT(VkPhysicalDevice physicalDevice, Display* dpy, RROutput rrOutput, VkDisplayKHR* pDisplay) const {
	return _table.vkGetRandROutputDisplayEXT(physicalDevice, dpy, rrOutput, pDisplay);
}
#endif /* defined(VK_EXT_acquire_xlib_display) */
#if defined(VK_EXT_calibrated_timestamps)
inline VkResult GetPhysicalDeviceCalibrateableTimeDomainsEXT(VkPhysicalDevice physicalDevice, uint32_t* pTimeDomainCount, VkTimeDomainEXT* pTimeDomains) const {
	return _table.vkGetPhysicalDeviceCalibrateableTimeDomainsEXT(physicalDevice, pTimeDomainCount, pTimeDomains);
}
#endif /* defined(VK_EXT_calibrated_timestamps) */
#if defined(VK_EXT_debug_report)
inline VkResult CreateDebugReportCallbackEXT(const VkDebugReportCallbackCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugReportCallbackEXT* pCallback) const {
	return _table.vkCreateDebugReportCallbackEXT(_instance, pCreateInfo, pAllocator, pCallback);
}
inline VkResult Create(const VkDebugReportCallbackCreateInfoEXT* pCreateInfo, VkDebugReportCallbackEXT* pCallback) const {
	return _table.vkCreateDebugReportCallbackEXT(_instance, pCreateInfo, _allocator, pCallback);
}
inline VkResult Create(const VkDebugReportCallbackCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugReportCallbackEXT* pCallback) const {
	return _table.vkCreateDebugReportCallbackEXT(_instance, pCreateInfo, pAllocator, pCallback);
}
inline VkResult CreateDebugReportCallbackEXT(const VkDebugReportCallbackCreateInfoEXT* pCreateInfo, VkDebugReportCallbackEXT* pCallback) const {
	return _table.vkCreateDebugReportCallbackEXT(_instance, pCreateInfo, _allocator, pCallback);
}
inline void DebugReportMessageEXT(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location, int32_t messageCode, const char* pLayerPrefix, const char* pMessage) const {
	_table.vkDebugReportMessageEXT(_instance, flags, objectType, object, location, messageCode, pLayerPrefix, pMessage);
}
inline void Destroy(VkDebugReportCallbackEXT callback) const {
	_table.vkDestroyDebugReportCallbackEXT(_instance, callback, _allocator);
}
inline void Destroy(VkDebugReportCallbackEXT callback, const VkAllocationCallbacks* pAllocator) const {
	_table.vkDestroyDebugReportCallbackEXT(_instance, callback, pAllocator);
}
inline void DestroyDebugReportCallbackEXT(VkDebugReportCallbackEXT callback) const {
	_table.vkDestroyDebugReportCallbackEXT(_instance, callback, _allocator);
}
inline void DestroyDebugReportCallbackEXT(VkDebugReportCallbackEXT callback, const VkAllocationCallbacks* pAllocator) const {
	_table.vkDestroyDebugReportCallbackEXT(_instance, callback, pAllocator);
}
#endif /* defined(VK_EXT_debug_report) */
#if defined(VK_EXT_debug_utils)
inline void CmdBeginDebugUtilsLabelEXT(VkCommandBuffer commandBuffer, const VkDebugUtilsLabelEXT* pLabelInfo) const {
	_table.vkCmdBeginDebugUtilsLabelEXT(commandBuffer, pLabelInfo);
}
inline void CmdEndDebugUtilsLabelEXT(VkCommandBuffer commandBuffer) const {
	_table.vkCmdEndDebugUtilsLabelEXT(commandBuffer);
}
inline void CmdInsertDebugUtilsLabelEXT(VkCommandBuffer commandBuffer, const VkDebugUtilsLabelEXT* pLabelInfo) const {
	_table.vkCmdInsertDebugUtilsLabelEXT(commandBuffer, pLabelInfo);
}
inline VkResult Create(const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pMessenger) const {
	return _table.vkCreateDebugUtilsMessengerEXT(_instance, pCreateInfo, pAllocator, pMessenger);
}
inline VkResult CreateDebugUtilsMessengerEXT(const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pMessenger) const {
	return _table.vkCreateDebugUtilsMessengerEXT(_instance, pCreateInfo, pAllocator, pMessenger);
}
inline VkResult CreateDebugUtilsMessengerEXT(const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, VkDebugUtilsMessengerEXT* pMessenger) const {
	return _table.vkCreateDebugUtilsMessengerEXT(_instance, pCreateInfo, _allocator, pMessenger);
}
inline VkResult Create(const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, VkDebugUtilsMessengerEXT* pMessenger) const {
	return _table.vkCreateDebugUtilsMessengerEXT(_instance, pCreateInfo, _allocator, pMessenger);
}
inline void Destroy(VkDebugUtilsMessengerEXT messenger) const {
	_table.vkDestroyDebugUtilsMessengerEXT(_instance, messenger, _allocator);
}
inline void DestroyDebugUtilsMessengerEXT(VkDebugUtilsMessengerEXT messenger) const {
	_table.vkDestroyDebugUtilsMessengerEXT(_instance, messenger, _allocator);
}
inline void Destroy(VkDebugUtilsMessengerEXT messenger, const VkAllocationCallbacks* pAllocator) const {
	_table.vkDestroyDebugUtilsMessengerEXT(_instance, messenger, pAllocator);
}
inline void DestroyDebugUtilsMessengerEXT(VkDebugUtilsMessengerEXT messenger, const VkAllocationCallbacks* pAllocator) const {
	_table.vkDestroyDebugUtilsMessengerEXT(_instance, messenger, pAllocator);
}
inline void QueueBeginDebugUtilsLabelEXT(VkQueue queue, const VkDebugUtilsLabelEXT* pLabelInfo) const {
	_table.vkQueueBeginDebugUtilsLabelEXT(queue, pLabelInfo);
}
inline void QueueEndDebugUtilsLabelEXT(VkQueue queue) const {
	_table.vkQueueEndDebugUtilsLabelEXT(queue);
}
inline void QueueInsertDebugUtilsLabelEXT(VkQueue queue, const VkDebugUtilsLabelEXT* pLabelInfo) const {
	_table.vkQueueInsertDebugUtilsLabelEXT(queue, pLabelInfo);
}
inline VkResult SetDebugUtilsObjectNameEXT(VkDevice device, const VkDebugUtilsObjectNameInfoEXT* pNameInfo) const {
	return _table.vkSetDebugUtilsObjectNameEXT(device, pNameInfo);
}
inline VkResult SetDebugUtilsObjectTagEXT(VkDevice device, const VkDebugUtilsObjectTagInfoEXT* pTagInfo) const {
	return _table.vkSetDebugUtilsObjectTagEXT(device, pTagInfo);
}
inline void SubmitDebugUtilsMessageEXT(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageTypes, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData) const {
	_table.vkSubmitDebugUtilsMessageEXT(_instance, messageSeverity, messageTypes, pCallbackData);
}
#endif /* defined(VK_EXT_debug_utils) */
#if defined(VK_EXT_direct_mode_display)
inline VkResult ReleaseDisplayEXT(VkPhysicalDevice physicalDevice, VkDisplayKHR display) const {
	return _table.vkReleaseDisplayEXT(physicalDevice, display);
}
#endif /* defined(VK_EXT_direct_mode_display) */
#if defined(VK_EXT_directfb_surface)
inline VkResult Create(const VkDirectFBSurfaceCreateInfoEXT* pCreateInfo, VkSurfaceKHR* pSurface) const {
	return _table.vkCreateDirectFBSurfaceEXT(_instance, pCreateInfo, _allocator, pSurface);
}
inline VkResult CreateDirectFBSurfaceEXT(const VkDirectFBSurfaceCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) const {
	return _table.vkCreateDirectFBSurfaceEXT(_instance, pCreateInfo, pAllocator, pSurface);
}
inline VkResult Create(const VkDirectFBSurfaceCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) const {
	return _table.vkCreateDirectFBSurfaceEXT(_instance, pCreateInfo, pAllocator, pSurface);
}
inline VkResult CreateDirectFBSurfaceEXT(const VkDirectFBSurfaceCreateInfoEXT* pCreateInfo, VkSurfaceKHR* pSurface) const {
	return _table.vkCreateDirectFBSurfaceEXT(_instance, pCreateInfo, _allocator, pSurface);
}
inline VkBool32 GetPhysicalDeviceDirectFBPresentationSupportEXT(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, IDirectFB* dfb) const {
	return _table.vkGetPhysicalDeviceDirectFBPresentationSupportEXT(physicalDevice, queueFamilyIndex, dfb);
}
#endif /* defined(VK_EXT_directfb_surface) */
#if defined(VK_EXT_display_surface_counter)
inline VkResult GetPhysicalDeviceSurfaceCapabilities2EXT(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkSurfaceCapabilities2EXT* pSurfaceCapabilities) const {
	return _table.vkGetPhysicalDeviceSurfaceCapabilities2EXT(physicalDevice, surface, pSurfaceCapabilities);
}
#endif /* defined(VK_EXT_display_surface_counter) */
#if defined(VK_EXT_full_screen_exclusive)
inline VkResult GetPhysicalDeviceSurfacePresentModes2EXT(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo, uint32_t* pPresentModeCount, VkPresentModeKHR* pPresentModes) const {
	return _table.vkGetPhysicalDeviceSurfacePresentModes2EXT(physicalDevice, pSurfaceInfo, pPresentModeCount, pPresentModes);
}
#endif /* defined(VK_EXT_full_screen_exclusive) */
#if defined(VK_EXT_headless_surface)
inline VkResult CreateHeadlessSurfaceEXT(const VkHeadlessSurfaceCreateInfoEXT* pCreateInfo, VkSurfaceKHR* pSurface) const {
	return _table.vkCreateHeadlessSurfaceEXT(_instance, pCreateInfo, _allocator, pSurface);
}
inline VkResult CreateHeadlessSurfaceEXT(const VkHeadlessSurfaceCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) const {
	return _table.vkCreateHeadlessSurfaceEXT(_instance, pCreateInfo, pAllocator, pSurface);
}
inline VkResult Create(const VkHeadlessSurfaceCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) const {
	return _table.vkCreateHeadlessSurfaceEXT(_instance, pCreateInfo, pAllocator, pSurface);
}
inline VkResult Create(const VkHeadlessSurfaceCreateInfoEXT* pCreateInfo, VkSurfaceKHR* pSurface) const {
	return _table.vkCreateHeadlessSurfaceEXT(_instance, pCreateInfo, _allocator, pSurface);
}
#endif /* defined(VK_EXT_headless_surface) */
#if defined(VK_EXT_metal_surface)
inline VkResult Create(const VkMetalSurfaceCreateInfoEXT* pCreateInfo, VkSurfaceKHR* pSurface) const {
	return _table.vkCreateMetalSurfaceEXT(_instance, pCreateInfo, _allocator, pSurface);
}
inline VkResult CreateMetalSurfaceEXT(const VkMetalSurfaceCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) const {
	return _table.vkCreateMetalSurfaceEXT(_instance, pCreateInfo, pAllocator, pSurface);
}
inline VkResult Create(const VkMetalSurfaceCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) const {
	return _table.vkCreateMetalSurfaceEXT(_instance, pCreateInfo, pAllocator, pSurface);
}
inline VkResult CreateMetalSurfaceEXT(const VkMetalSurfaceCreateInfoEXT* pCreateInfo, VkSurfaceKHR* pSurface) const {
	return _table.vkCreateMetalSurfaceEXT(_instance, pCreateInfo, _allocator, pSurface);
}
#endif /* defined(VK_EXT_metal_surface) */
#if defined(VK_EXT_sample_locations)
inline void GetPhysicalDeviceMultisamplePropertiesEXT(VkPhysicalDevice physicalDevice, VkSampleCountFlagBits samples, VkMultisamplePropertiesEXT* pMultisampleProperties) const {
	_table.vkGetPhysicalDeviceMultisamplePropertiesEXT(physicalDevice, samples, pMultisampleProperties);
}
#endif /* defined(VK_EXT_sample_locations) */
#if defined(VK_EXT_tooling_info)
inline VkResult GetPhysicalDeviceToolPropertiesEXT(VkPhysicalDevice physicalDevice, uint32_t* pToolCount, VkPhysicalDeviceToolProperties* pToolProperties) const {
	return _table.vkGetPhysicalDeviceToolPropertiesEXT(physicalDevice, pToolCount, pToolProperties);
}
#endif /* defined(VK_EXT_tooling_info) */
#if defined(VK_FUCHSIA_imagepipe_surface)
inline VkResult Create(const VkImagePipeSurfaceCreateInfoFUCHSIA* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) const {
	return _table.vkCreateImagePipeSurfaceFUCHSIA(_instance, pCreateInfo, pAllocator, pSurface);
}
inline VkResult CreateImagePipeSurfaceFUCHSIA(const VkImagePipeSurfaceCreateInfoFUCHSIA* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) const {
	return _table.vkCreateImagePipeSurfaceFUCHSIA(_instance, pCreateInfo, pAllocator, pSurface);
}
inline VkResult Create(const VkImagePipeSurfaceCreateInfoFUCHSIA* pCreateInfo, VkSurfaceKHR* pSurface) const {
	return _table.vkCreateImagePipeSurfaceFUCHSIA(_instance, pCreateInfo, _allocator, pSurface);
}
inline VkResult CreateImagePipeSurfaceFUCHSIA(const VkImagePipeSurfaceCreateInfoFUCHSIA* pCreateInfo, VkSurfaceKHR* pSurface) const {
	return _table.vkCreateImagePipeSurfaceFUCHSIA(_instance, pCreateInfo, _allocator, pSurface);
}
#endif /* defined(VK_FUCHSIA_imagepipe_surface) */
#if defined(VK_GGP_stream_descriptor_surface)
inline VkResult CreateStreamDescriptorSurfaceGGP(const VkStreamDescriptorSurfaceCreateInfoGGP* pCreateInfo, VkSurfaceKHR* pSurface) const {
	return _table.vkCreateStreamDescriptorSurfaceGGP(_instance, pCreateInfo, _allocator, pSurface);
}
inline VkResult Create(const VkStreamDescriptorSurfaceCreateInfoGGP* pCreateInfo, VkSurfaceKHR* pSurface) const {
	return _table.vkCreateStreamDescriptorSurfaceGGP(_instance, pCreateInfo, _allocator, pSurface);
}
inline VkResult CreateStreamDescriptorSurfaceGGP(const VkStreamDescriptorSurfaceCreateInfoGGP* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) const {
	return _table.vkCreateStreamDescriptorSurfaceGGP(_instance, pCreateInfo, pAllocator, pSurface);
}
inline VkResult Create(const VkStreamDescriptorSurfaceCreateInfoGGP* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) const {
	return _table.vkCreateStreamDescriptorSurfaceGGP(_instance, pCreateInfo, pAllocator, pSurface);
}
#endif /* defined(VK_GGP_stream_descriptor_surface) */
#if defined(VK_KHR_android_surface)
inline VkResult Create(const VkAndroidSurfaceCreateInfoKHR* pCreateInfo, VkSurfaceKHR* pSurface) const {
	return _table.vkCreateAndroidSurfaceKHR(_instance, pCreateInfo, _allocator, pSurface);
}
inline VkResult CreateAndroidSurfaceKHR(const VkAndroidSurfaceCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) const {
	return _table.vkCreateAndroidSurfaceKHR(_instance, pCreateInfo, pAllocator, pSurface);
}
inline VkResult Create(const VkAndroidSurfaceCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) const {
	return _table.vkCreateAndroidSurfaceKHR(_instance, pCreateInfo, pAllocator, pSurface);
}
inline VkResult CreateAndroidSurfaceKHR(const VkAndroidSurfaceCreateInfoKHR* pCreateInfo, VkSurfaceKHR* pSurface) const {
	return _table.vkCreateAndroidSurfaceKHR(_instance, pCreateInfo, _allocator, pSurface);
}
#endif /* defined(VK_KHR_android_surface) */
#if defined(VK_KHR_device_group_creation)
inline VkResult EnumeratePhysicalDeviceGroupsKHR(uint32_t* pPhysicalDeviceGroupCount, VkPhysicalDeviceGroupProperties* pPhysicalDeviceGroupProperties) const {
	return _table.vkEnumeratePhysicalDeviceGroupsKHR(_instance, pPhysicalDeviceGroupCount, pPhysicalDeviceGroupProperties);
}
#endif /* defined(VK_KHR_device_group_creation) */
#if defined(VK_KHR_display)
inline VkResult Create(VkPhysicalDevice physicalDevice, VkDisplayKHR display, const VkDisplayModeCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDisplayModeKHR* pMode) const {
	return _table.vkCreateDisplayModeKHR(physicalDevice, display, pCreateInfo, pAllocator, pMode);
}
inline VkResult CreateDisplayModeKHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display, const VkDisplayModeCreateInfoKHR* pCreateInfo, VkDisplayModeKHR* pMode) const {
	return _table.vkCreateDisplayModeKHR(physicalDevice, display, pCreateInfo, _allocator, pMode);
}
inline VkResult CreateDisplayModeKHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display, const VkDisplayModeCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDisplayModeKHR* pMode) const {
	return _table.vkCreateDisplayModeKHR(physicalDevice, display, pCreateInfo, pAllocator, pMode);
}
inline VkResult Create(VkPhysicalDevice physicalDevice, VkDisplayKHR display, const VkDisplayModeCreateInfoKHR* pCreateInfo, VkDisplayModeKHR* pMode) const {
	return _table.vkCreateDisplayModeKHR(physicalDevice, display, pCreateInfo, _allocator, pMode);
}
inline VkResult Create(const VkDisplaySurfaceCreateInfoKHR* pCreateInfo, VkSurfaceKHR* pSurface) const {
	return _table.vkCreateDisplayPlaneSurfaceKHR(_instance, pCreateInfo, _allocator, pSurface);
}
inline VkResult CreateDisplayPlaneSurfaceKHR(const VkDisplaySurfaceCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) const {
	return _table.vkCreateDisplayPlaneSurfaceKHR(_instance, pCreateInfo, pAllocator, pSurface);
}
inline VkResult Create(const VkDisplaySurfaceCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) const {
	return _table.vkCreateDisplayPlaneSurfaceKHR(_instance, pCreateInfo, pAllocator, pSurface);
}
inline VkResult CreateDisplayPlaneSurfaceKHR(const VkDisplaySurfaceCreateInfoKHR* pCreateInfo, VkSurfaceKHR* pSurface) const {
	return _table.vkCreateDisplayPlaneSurfaceKHR(_instance, pCreateInfo, _allocator, pSurface);
}
inline VkResult GetDisplayModePropertiesKHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display, uint32_t* pPropertyCount, VkDisplayModePropertiesKHR* pProperties) const {
	return _table.vkGetDisplayModePropertiesKHR(physicalDevice, display, pPropertyCount, pProperties);
}
inline VkResult GetDisplayPlaneCapabilitiesKHR(VkPhysicalDevice physicalDevice, VkDisplayModeKHR mode, uint32_t planeIndex, VkDisplayPlaneCapabilitiesKHR* pCapabilities) const {
	return _table.vkGetDisplayPlaneCapabilitiesKHR(physicalDevice, mode, planeIndex, pCapabilities);
}
inline VkResult GetDisplayPlaneSupportedDisplaysKHR(VkPhysicalDevice physicalDevice, uint32_t planeIndex, uint32_t* pDisplayCount, VkDisplayKHR* pDisplays) const {
	return _table.vkGetDisplayPlaneSupportedDisplaysKHR(physicalDevice, planeIndex, pDisplayCount, pDisplays);
}
inline VkResult GetPhysicalDeviceDisplayPlanePropertiesKHR(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount, VkDisplayPlanePropertiesKHR* pProperties) const {
	return _table.vkGetPhysicalDeviceDisplayPlanePropertiesKHR(physicalDevice, pPropertyCount, pProperties);
}
inline VkResult GetPhysicalDeviceDisplayPropertiesKHR(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount, VkDisplayPropertiesKHR* pProperties) const {
	return _table.vkGetPhysicalDeviceDisplayPropertiesKHR(physicalDevice, pPropertyCount, pProperties);
}
#endif /* defined(VK_KHR_display) */
#if defined(VK_KHR_external_fence_capabilities)
inline void GetPhysicalDeviceExternalFencePropertiesKHR(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalFenceInfo* pExternalFenceInfo, VkExternalFenceProperties* pExternalFenceProperties) const {
	_table.vkGetPhysicalDeviceExternalFencePropertiesKHR(physicalDevice, pExternalFenceInfo, pExternalFenceProperties);
}
#endif /* defined(VK_KHR_external_fence_capabilities) */
#if defined(VK_KHR_external_memory_capabilities)
inline void GetPhysicalDeviceExternalBufferPropertiesKHR(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalBufferInfo* pExternalBufferInfo, VkExternalBufferProperties* pExternalBufferProperties) const {
	_table.vkGetPhysicalDeviceExternalBufferPropertiesKHR(physicalDevice, pExternalBufferInfo, pExternalBufferProperties);
}
#endif /* defined(VK_KHR_external_memory_capabilities) */
#if defined(VK_KHR_external_semaphore_capabilities)
inline void GetPhysicalDeviceExternalSemaphorePropertiesKHR(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalSemaphoreInfo* pExternalSemaphoreInfo, VkExternalSemaphoreProperties* pExternalSemaphoreProperties) const {
	_table.vkGetPhysicalDeviceExternalSemaphorePropertiesKHR(physicalDevice, pExternalSemaphoreInfo, pExternalSemaphoreProperties);
}
#endif /* defined(VK_KHR_external_semaphore_capabilities) */
#if defined(VK_KHR_fragment_shading_rate)
inline VkResult GetPhysicalDeviceFragmentShadingRatesKHR(VkPhysicalDevice physicalDevice, uint32_t* pFragmentShadingRateCount, VkPhysicalDeviceFragmentShadingRateKHR* pFragmentShadingRates) const {
	return _table.vkGetPhysicalDeviceFragmentShadingRatesKHR(physicalDevice, pFragmentShadingRateCount, pFragmentShadingRates);
}
#endif /* defined(VK_KHR_fragment_shading_rate) */
#if defined(VK_KHR_get_display_properties2)
inline VkResult GetDisplayModeProperties2KHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display, uint32_t* pPropertyCount, VkDisplayModeProperties2KHR* pProperties) const {
	return _table.vkGetDisplayModeProperties2KHR(physicalDevice, display, pPropertyCount, pProperties);
}
inline VkResult GetDisplayPlaneCapabilities2KHR(VkPhysicalDevice physicalDevice, const VkDisplayPlaneInfo2KHR* pDisplayPlaneInfo, VkDisplayPlaneCapabilities2KHR* pCapabilities) const {
	return _table.vkGetDisplayPlaneCapabilities2KHR(physicalDevice, pDisplayPlaneInfo, pCapabilities);
}
inline VkResult GetPhysicalDeviceDisplayPlaneProperties2KHR(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount, VkDisplayPlaneProperties2KHR* pProperties) const {
	return _table.vkGetPhysicalDeviceDisplayPlaneProperties2KHR(physicalDevice, pPropertyCount, pProperties);
}
inline VkResult GetPhysicalDeviceDisplayProperties2KHR(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount, VkDisplayProperties2KHR* pProperties) const {
	return _table.vkGetPhysicalDeviceDisplayProperties2KHR(physicalDevice, pPropertyCount, pProperties);
}
#endif /* defined(VK_KHR_get_display_properties2) */
#if defined(VK_KHR_get_physical_device_properties2)
inline void GetPhysicalDeviceFeatures2KHR(VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures2* pFeatures) const {
	_table.vkGetPhysicalDeviceFeatures2KHR(physicalDevice, pFeatures);
}
inline void GetPhysicalDeviceFormatProperties2KHR(VkPhysicalDevice physicalDevice, VkFormat format, VkFormatProperties2* pFormatProperties) const {
	_table.vkGetPhysicalDeviceFormatProperties2KHR(physicalDevice, format, pFormatProperties);
}
inline VkResult GetPhysicalDeviceImageFormatProperties2KHR(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceImageFormatInfo2* pImageFormatInfo, VkImageFormatProperties2* pImageFormatProperties) const {
	return _table.vkGetPhysicalDeviceImageFormatProperties2KHR(physicalDevice, pImageFormatInfo, pImageFormatProperties);
}
inline void GetPhysicalDeviceMemoryProperties2KHR(VkPhysicalDevice physicalDevice, VkPhysicalDeviceMemoryProperties2* pMemoryProperties) const {
	_table.vkGetPhysicalDeviceMemoryProperties2KHR(physicalDevice, pMemoryProperties);
}
inline void GetPhysicalDeviceProperties2KHR(VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties2* pProperties) const {
	_table.vkGetPhysicalDeviceProperties2KHR(physicalDevice, pProperties);
}
inline void GetPhysicalDeviceQueueFamilyProperties2KHR(VkPhysicalDevice physicalDevice, uint32_t* pQueueFamilyPropertyCount, VkQueueFamilyProperties2* pQueueFamilyProperties) const {
	_table.vkGetPhysicalDeviceQueueFamilyProperties2KHR(physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties);
}
inline void GetPhysicalDeviceSparseImageFormatProperties2KHR(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceSparseImageFormatInfo2* pFormatInfo, uint32_t* pPropertyCount, VkSparseImageFormatProperties2* pProperties) const {
	_table.vkGetPhysicalDeviceSparseImageFormatProperties2KHR(physicalDevice, pFormatInfo, pPropertyCount, pProperties);
}
#endif /* defined(VK_KHR_get_physical_device_properties2) */
#if defined(VK_KHR_get_surface_capabilities2)
inline VkResult GetPhysicalDeviceSurfaceCapabilities2KHR(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo, VkSurfaceCapabilities2KHR* pSurfaceCapabilities) const {
	return _table.vkGetPhysicalDeviceSurfaceCapabilities2KHR(physicalDevice, pSurfaceInfo, pSurfaceCapabilities);
}
inline VkResult GetPhysicalDeviceSurfaceFormats2KHR(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo, uint32_t* pSurfaceFormatCount, VkSurfaceFormat2KHR* pSurfaceFormats) const {
	return _table.vkGetPhysicalDeviceSurfaceFormats2KHR(physicalDevice, pSurfaceInfo, pSurfaceFormatCount, pSurfaceFormats);
}
#endif /* defined(VK_KHR_get_surface_capabilities2) */
#if defined(VK_KHR_performance_query)
inline VkResult EnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, uint32_t* pCounterCount, VkPerformanceCounterKHR* pCounters, VkPerformanceCounterDescriptionKHR* pCounterDescriptions) const {
	return _table.vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(physicalDevice, queueFamilyIndex, pCounterCount, pCounters, pCounterDescriptions);
}
inline void GetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR(VkPhysicalDevice physicalDevice, const VkQueryPoolPerformanceCreateInfoKHR* pPerformanceQueryCreateInfo, uint32_t* pNumPasses) const {
	_table.vkGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR(physicalDevice, pPerformanceQueryCreateInfo, pNumPasses);
}
#endif /* defined(VK_KHR_performance_query) */
#if defined(VK_KHR_surface)
inline void Destroy(VkSurfaceKHR surface, const VkAllocationCallbacks* pAllocator) const {
	_table.vkDestroySurfaceKHR(_instance, surface, pAllocator);
}
inline void Destroy(VkSurfaceKHR surface) const {
	_table.vkDestroySurfaceKHR(_instance, surface, _allocator);
}
inline void DestroySurfaceKHR(VkSurfaceKHR surface) const {
	_table.vkDestroySurfaceKHR(_instance, surface, _allocator);
}
inline void DestroySurfaceKHR(VkSurfaceKHR surface, const VkAllocationCallbacks* pAllocator) const {
	_table.vkDestroySurfaceKHR(_instance, surface, pAllocator);
}
inline VkResult GetPhysicalDeviceSurfaceCapabilitiesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkSurfaceCapabilitiesKHR* pSurfaceCapabilities) const {
	return _table.vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, pSurfaceCapabilities);
}
inline VkResult GetPhysicalDeviceSurfaceFormatsKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, uint32_t* pSurfaceFormatCount, VkSurfaceFormatKHR* pSurfaceFormats) const {
	return _table.vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, pSurfaceFormatCount, pSurfaceFormats);
}
inline VkResult GetPhysicalDeviceSurfacePresentModesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, uint32_t* pPresentModeCount, VkPresentModeKHR* pPresentModes) const {
	return _table.vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, pPresentModeCount, pPresentModes);
}
inline VkResult GetPhysicalDeviceSurfaceSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, VkSurfaceKHR surface, VkBool32* pSupported) const {
	return _table.vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, queueFamilyIndex, surface, pSupported);
}
#endif /* defined(VK_KHR_surface) */
#if defined(VK_KHR_video_queue)
inline VkResult GetPhysicalDeviceVideoCapabilitiesKHR(VkPhysicalDevice physicalDevice, const VkVideoProfileKHR* pVideoProfile, VkVideoCapabilitiesKHR* pCapabilities) const {
	return _table.vkGetPhysicalDeviceVideoCapabilitiesKHR(physicalDevice, pVideoProfile, pCapabilities);
}
inline VkResult GetPhysicalDeviceVideoFormatPropertiesKHR(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceVideoFormatInfoKHR* pVideoFormatInfo, uint32_t* pVideoFormatPropertyCount, VkVideoFormatPropertiesKHR* pVideoFormatProperties) const {
	return _table.vkGetPhysicalDeviceVideoFormatPropertiesKHR(physicalDevice, pVideoFormatInfo, pVideoFormatPropertyCount, pVideoFormatProperties);
}
#endif /* defined(VK_KHR_video_queue) */
#if defined(VK_KHR_wayland_surface)
inline VkResult Create(const VkWaylandSurfaceCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) const {
	return _table.vkCreateWaylandSurfaceKHR(_instance, pCreateInfo, pAllocator, pSurface);
}
inline VkResult CreateWaylandSurfaceKHR(const VkWaylandSurfaceCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) const {
	return _table.vkCreateWaylandSurfaceKHR(_instance, pCreateInfo, pAllocator, pSurface);
}
inline VkResult Create(const VkWaylandSurfaceCreateInfoKHR* pCreateInfo, VkSurfaceKHR* pSurface) const {
	return _table.vkCreateWaylandSurfaceKHR(_instance, pCreateInfo, _allocator, pSurface);
}
inline VkResult CreateWaylandSurfaceKHR(const VkWaylandSurfaceCreateInfoKHR* pCreateInfo, VkSurfaceKHR* pSurface) const {
	return _table.vkCreateWaylandSurfaceKHR(_instance, pCreateInfo, _allocator, pSurface);
}
inline VkBool32 GetPhysicalDeviceWaylandPresentationSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, struct wl_display* display) const {
	return _table.vkGetPhysicalDeviceWaylandPresentationSupportKHR(physicalDevice, queueFamilyIndex, display);
}
#endif /* defined(VK_KHR_wayland_surface) */
#if defined(VK_KHR_win32_surface)
inline VkResult Create(const VkWin32SurfaceCreateInfoKHR* pCreateInfo, VkSurfaceKHR* pSurface) const {
	return _table.vkCreateWin32SurfaceKHR(_instance, pCreateInfo, _allocator, pSurface);
}
inline VkResult CreateWin32SurfaceKHR(const VkWin32SurfaceCreateInfoKHR* pCreateInfo, VkSurfaceKHR* pSurface) const {
	return _table.vkCreateWin32SurfaceKHR(_instance, pCreateInfo, _allocator, pSurface);
}
inline VkResult Create(const VkWin32SurfaceCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) const {
	return _table.vkCreateWin32SurfaceKHR(_instance, pCreateInfo, pAllocator, pSurface);
}
inline VkResult CreateWin32SurfaceKHR(const VkWin32SurfaceCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) const {
	return _table.vkCreateWin32SurfaceKHR(_instance, pCreateInfo, pAllocator, pSurface);
}
inline VkBool32 GetPhysicalDeviceWin32PresentationSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex) const {
	return _table.vkGetPhysicalDeviceWin32PresentationSupportKHR(physicalDevice, queueFamilyIndex);
}
#endif /* defined(VK_KHR_win32_surface) */
#if defined(VK_KHR_xcb_surface)
inline VkResult Create(const VkXcbSurfaceCreateInfoKHR* pCreateInfo, VkSurfaceKHR* pSurface) const {
	return _table.vkCreateXcbSurfaceKHR(_instance, pCreateInfo, _allocator, pSurface);
}
inline VkResult Create(const VkXcbSurfaceCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) const {
	return _table.vkCreateXcbSurfaceKHR(_instance, pCreateInfo, pAllocator, pSurface);
}
inline VkResult CreateXcbSurfaceKHR(const VkXcbSurfaceCreateInfoKHR* pCreateInfo, VkSurfaceKHR* pSurface) const {
	return _table.vkCreateXcbSurfaceKHR(_instance, pCreateInfo, _allocator, pSurface);
}
inline VkResult CreateXcbSurfaceKHR(const VkXcbSurfaceCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) const {
	return _table.vkCreateXcbSurfaceKHR(_instance, pCreateInfo, pAllocator, pSurface);
}
inline VkBool32 GetPhysicalDeviceXcbPresentationSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, xcb_connection_t* connection, xcb_visualid_t visual_id) const {
	return _table.vkGetPhysicalDeviceXcbPresentationSupportKHR(physicalDevice, queueFamilyIndex, connection, visual_id);
}
#endif /* defined(VK_KHR_xcb_surface) */
#if defined(VK_KHR_xlib_surface)
inline VkResult Create(const VkXlibSurfaceCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) const {
	return _table.vkCreateXlibSurfaceKHR(_instance, pCreateInfo, pAllocator, pSurface);
}
inline VkResult CreateXlibSurfaceKHR(const VkXlibSurfaceCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) const {
	return _table.vkCreateXlibSurfaceKHR(_instance, pCreateInfo, pAllocator, pSurface);
}
inline VkResult Create(const VkXlibSurfaceCreateInfoKHR* pCreateInfo, VkSurfaceKHR* pSurface) const {
	return _table.vkCreateXlibSurfaceKHR(_instance, pCreateInfo, _allocator, pSurface);
}
inline VkResult CreateXlibSurfaceKHR(const VkXlibSurfaceCreateInfoKHR* pCreateInfo, VkSurfaceKHR* pSurface) const {
	return _table.vkCreateXlibSurfaceKHR(_instance, pCreateInfo, _allocator, pSurface);
}
inline VkBool32 GetPhysicalDeviceXlibPresentationSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, Display* dpy, VisualID visualID) const {
	return _table.vkGetPhysicalDeviceXlibPresentationSupportKHR(physicalDevice, queueFamilyIndex, dpy, visualID);
}
#endif /* defined(VK_KHR_xlib_surface) */
#if defined(VK_MVK_ios_surface)
inline VkResult Create(const VkIOSSurfaceCreateInfoMVK* pCreateInfo, VkSurfaceKHR* pSurface) const {
	return _table.vkCreateIOSSurfaceMVK(_instance, pCreateInfo, _allocator, pSurface);
}
inline VkResult CreateIOSSurfaceMVK(const VkIOSSurfaceCreateInfoMVK* pCreateInfo, VkSurfaceKHR* pSurface) const {
	return _table.vkCreateIOSSurfaceMVK(_instance, pCreateInfo, _allocator, pSurface);
}
inline VkResult CreateIOSSurfaceMVK(const VkIOSSurfaceCreateInfoMVK* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) const {
	return _table.vkCreateIOSSurfaceMVK(_instance, pCreateInfo, pAllocator, pSurface);
}
inline VkResult Create(const VkIOSSurfaceCreateInfoMVK* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) const {
	return _table.vkCreateIOSSurfaceMVK(_instance, pCreateInfo, pAllocator, pSurface);
}
#endif /* defined(VK_MVK_ios_surface) */
#if defined(VK_MVK_macos_surface)
inline VkResult CreateMacOSSurfaceMVK(const VkMacOSSurfaceCreateInfoMVK* pCreateInfo, VkSurfaceKHR* pSurface) const {
	return _table.vkCreateMacOSSurfaceMVK(_instance, pCreateInfo, _allocator, pSurface);
}
inline VkResult Create(const VkMacOSSurfaceCreateInfoMVK* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) const {
	return _table.vkCreateMacOSSurfaceMVK(_instance, pCreateInfo, pAllocator, pSurface);
}
inline VkResult CreateMacOSSurfaceMVK(const VkMacOSSurfaceCreateInfoMVK* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) const {
	return _table.vkCreateMacOSSurfaceMVK(_instance, pCreateInfo, pAllocator, pSurface);
}
inline VkResult Create(const VkMacOSSurfaceCreateInfoMVK* pCreateInfo, VkSurfaceKHR* pSurface) const {
	return _table.vkCreateMacOSSurfaceMVK(_instance, pCreateInfo, _allocator, pSurface);
}
#endif /* defined(VK_MVK_macos_surface) */
#if defined(VK_NN_vi_surface)
inline VkResult CreateViSurfaceNN(const VkViSurfaceCreateInfoNN* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) const {
	return _table.vkCreateViSurfaceNN(_instance, pCreateInfo, pAllocator, pSurface);
}
inline VkResult Create(const VkViSurfaceCreateInfoNN* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) const {
	return _table.vkCreateViSurfaceNN(_instance, pCreateInfo, pAllocator, pSurface);
}
inline VkResult CreateViSurfaceNN(const VkViSurfaceCreateInfoNN* pCreateInfo, VkSurfaceKHR* pSurface) const {
	return _table.vkCreateViSurfaceNN(_instance, pCreateInfo, _allocator, pSurface);
}
inline VkResult Create(const VkViSurfaceCreateInfoNN* pCreateInfo, VkSurfaceKHR* pSurface) const {
	return _table.vkCreateViSurfaceNN(_instance, pCreateInfo, _allocator, pSurface);
}
#endif /* defined(VK_NN_vi_surface) */
#if defined(VK_NV_acquire_winrt_display)
inline VkResult AcquireWinrtDisplayNV(VkPhysicalDevice physicalDevice, VkDisplayKHR display) const {
	return _table.vkAcquireWinrtDisplayNV(physicalDevice, display);
}
inline VkResult GetWinrtDisplayNV(VkPhysicalDevice physicalDevice, uint32_t deviceRelativeId, VkDisplayKHR* pDisplay) const {
	return _table.vkGetWinrtDisplayNV(physicalDevice, deviceRelativeId, pDisplay);
}
#endif /* defined(VK_NV_acquire_winrt_display) */
#if defined(VK_NV_cooperative_matrix)
inline VkResult GetPhysicalDeviceCooperativeMatrixPropertiesNV(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount, VkCooperativeMatrixPropertiesNV* pProperties) const {
	return _table.vkGetPhysicalDeviceCooperativeMatrixPropertiesNV(physicalDevice, pPropertyCount, pProperties);
}
#endif /* defined(VK_NV_cooperative_matrix) */
#if defined(VK_NV_coverage_reduction_mode)
inline VkResult GetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV(VkPhysicalDevice physicalDevice, uint32_t* pCombinationCount, VkFramebufferMixedSamplesCombinationNV* pCombinations) const {
	return _table.vkGetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV(physicalDevice, pCombinationCount, pCombinations);
}
#endif /* defined(VK_NV_coverage_reduction_mode) */
#if defined(VK_NV_external_memory_capabilities)
inline VkResult GetPhysicalDeviceExternalImageFormatPropertiesNV(VkPhysicalDevice physicalDevice, VkFormat format, VkImageType type, VkImageTiling tiling, VkImageUsageFlags usage, VkImageCreateFlags flags, VkExternalMemoryHandleTypeFlagsNV externalHandleType, VkExternalImageFormatPropertiesNV* pExternalImageFormatProperties) const {
	return _table.vkGetPhysicalDeviceExternalImageFormatPropertiesNV(physicalDevice, format, type, tiling, usage, flags, externalHandleType, pExternalImageFormatProperties);
}
#endif /* defined(VK_NV_external_memory_capabilities) */
#if defined(VK_QNX_screen_surface)
inline VkResult CreateScreenSurfaceQNX(const VkScreenSurfaceCreateInfoQNX* pCreateInfo, VkSurfaceKHR* pSurface) const {
	return _table.vkCreateScreenSurfaceQNX(_instance, pCreateInfo, _allocator, pSurface);
}
inline VkResult Create(const VkScreenSurfaceCreateInfoQNX* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) const {
	return _table.vkCreateScreenSurfaceQNX(_instance, pCreateInfo, pAllocator, pSurface);
}
inline VkResult CreateScreenSurfaceQNX(const VkScreenSurfaceCreateInfoQNX* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) const {
	return _table.vkCreateScreenSurfaceQNX(_instance, pCreateInfo, pAllocator, pSurface);
}
inline VkResult Create(const VkScreenSurfaceCreateInfoQNX* pCreateInfo, VkSurfaceKHR* pSurface) const {
	return _table.vkCreateScreenSurfaceQNX(_instance, pCreateInfo, _allocator, pSurface);
}
inline VkBool32 GetPhysicalDeviceScreenPresentationSupportQNX(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, struct _screen_window* window) const {
	return _table.vkGetPhysicalDeviceScreenPresentationSupportQNX(physicalDevice, queueFamilyIndex, window);
}
#endif /* defined(VK_QNX_screen_surface) */
#if (defined(VK_KHR_device_group) && defined(VK_KHR_surface)) || (defined(VK_KHR_swapchain) && defined(VK_VERSION_1_1))
inline VkResult GetPhysicalDevicePresentRectanglesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, uint32_t* pRectCount, VkRect2D* pRects) const {
	return _table.vkGetPhysicalDevicePresentRectanglesKHR(physicalDevice, surface, pRectCount, pRects);
}
#endif /* (defined(VK_KHR_device_group) && defined(VK_KHR_surface)) || (defined(VK_KHR_swapchain) && defined(VK_VERSION_1_1)) */
  /* VOLK_GENERATE_INSTANCE_METHOD_HPP */
};

class CommandBuffer {
protected:
  Device *_device;
  VkCommandBuffer _command_buffer;

public:
  CommandBuffer(Device *device, VkCommandBuffer command_buffer)
      : _device(device), _command_buffer(command_buffer) {}

  inline Device *device() const {
    return _device;
  }

  inline VkCommandBuffer command_buffer() const {
    return _command_buffer;
  }

  /* VOLK_GENERATE_COMMAND_BUFFER_METHOD_HPP */
#if defined(VK_VERSION_1_0)
inline void BeginQuery(VkQueryPool queryPool, uint32_t query, VkQueryControlFlags flags) const {
	_device->CmdBeginQuery(_command_buffer, queryPool, query, flags);
}
inline void BeginRenderPass(const VkRenderPassBeginInfo* pRenderPassBegin, VkSubpassContents contents) const {
	_device->CmdBeginRenderPass(_command_buffer, pRenderPassBegin, contents);
}
inline void BindDescriptorSets(VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout, uint32_t firstSet, uint32_t descriptorSetCount, const VkDescriptorSet* pDescriptorSets, uint32_t dynamicOffsetCount, const uint32_t* pDynamicOffsets) const {
	_device->CmdBindDescriptorSets(_command_buffer, pipelineBindPoint, layout, firstSet, descriptorSetCount, pDescriptorSets, dynamicOffsetCount, pDynamicOffsets);
}
inline void BindIndexBuffer(VkBuffer buffer, VkDeviceSize offset, VkIndexType indexType) const {
	_device->CmdBindIndexBuffer(_command_buffer, buffer, offset, indexType);
}
inline void BindPipeline(VkPipelineBindPoint pipelineBindPoint, VkPipeline pipeline) const {
	_device->CmdBindPipeline(_command_buffer, pipelineBindPoint, pipeline);
}
inline void BindVertexBuffers(uint32_t firstBinding, uint32_t bindingCount, const VkBuffer* pBuffers, const VkDeviceSize* pOffsets) const {
	_device->CmdBindVertexBuffers(_command_buffer, firstBinding, bindingCount, pBuffers, pOffsets);
}
inline void BlitImage(VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageBlit* pRegions, VkFilter filter) const {
	_device->CmdBlitImage(_command_buffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions, filter);
}
inline void ClearAttachments(uint32_t attachmentCount, const VkClearAttachment* pAttachments, uint32_t rectCount, const VkClearRect* pRects) const {
	_device->CmdClearAttachments(_command_buffer, attachmentCount, pAttachments, rectCount, pRects);
}
inline void ClearColorImage(VkImage image, VkImageLayout imageLayout, const VkClearColorValue* pColor, uint32_t rangeCount, const VkImageSubresourceRange* pRanges) const {
	_device->CmdClearColorImage(_command_buffer, image, imageLayout, pColor, rangeCount, pRanges);
}
inline void ClearDepthStencilImage(VkImage image, VkImageLayout imageLayout, const VkClearDepthStencilValue* pDepthStencil, uint32_t rangeCount, const VkImageSubresourceRange* pRanges) const {
	_device->CmdClearDepthStencilImage(_command_buffer, image, imageLayout, pDepthStencil, rangeCount, pRanges);
}
inline void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, uint32_t regionCount, const VkBufferCopy* pRegions) const {
	_device->CmdCopyBuffer(_command_buffer, srcBuffer, dstBuffer, regionCount, pRegions);
}
inline void CopyBufferToImage(VkBuffer srcBuffer, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkBufferImageCopy* pRegions) const {
	_device->CmdCopyBufferToImage(_command_buffer, srcBuffer, dstImage, dstImageLayout, regionCount, pRegions);
}
inline void CopyImage(VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageCopy* pRegions) const {
	_device->CmdCopyImage(_command_buffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions);
}
inline void CopyImageToBuffer(VkImage srcImage, VkImageLayout srcImageLayout, VkBuffer dstBuffer, uint32_t regionCount, const VkBufferImageCopy* pRegions) const {
	_device->CmdCopyImageToBuffer(_command_buffer, srcImage, srcImageLayout, dstBuffer, regionCount, pRegions);
}
inline void CopyQueryPoolResults(VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize stride, VkQueryResultFlags flags) const {
	_device->CmdCopyQueryPoolResults(_command_buffer, queryPool, firstQuery, queryCount, dstBuffer, dstOffset, stride, flags);
}
inline void Dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) const {
	_device->CmdDispatch(_command_buffer, groupCountX, groupCountY, groupCountZ);
}
inline void DispatchIndirect(VkBuffer buffer, VkDeviceSize offset) const {
	_device->CmdDispatchIndirect(_command_buffer, buffer, offset);
}
inline void Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) const {
	_device->CmdDraw(_command_buffer, vertexCount, instanceCount, firstVertex, firstInstance);
}
inline void DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance) const {
	_device->CmdDrawIndexed(_command_buffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}
inline void DrawIndexedIndirect(VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount, uint32_t stride) const {
	_device->CmdDrawIndexedIndirect(_command_buffer, buffer, offset, drawCount, stride);
}
inline void DrawIndirect(VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount, uint32_t stride) const {
	_device->CmdDrawIndirect(_command_buffer, buffer, offset, drawCount, stride);
}
inline void EndQuery(VkQueryPool queryPool, uint32_t query) const {
	_device->CmdEndQuery(_command_buffer, queryPool, query);
}
inline void EndRenderPass() const {
	_device->CmdEndRenderPass(_command_buffer);
}
inline void ExecuteCommands(uint32_t commandBufferCount, const VkCommandBuffer* pCommandBuffers) const {
	_device->CmdExecuteCommands(_command_buffer, commandBufferCount, pCommandBuffers);
}
inline void FillBuffer(VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize size, uint32_t data) const {
	_device->CmdFillBuffer(_command_buffer, dstBuffer, dstOffset, size, data);
}
inline void NextSubpass(VkSubpassContents contents) const {
	_device->CmdNextSubpass(_command_buffer, contents);
}
inline void PipelineBarrier(VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags, uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers, uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier* pImageMemoryBarriers) const {
	_device->CmdPipelineBarrier(_command_buffer, srcStageMask, dstStageMask, dependencyFlags, memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
}
inline void PushConstants(VkPipelineLayout layout, VkShaderStageFlags stageFlags, uint32_t offset, uint32_t size, const void* pValues) const {
	_device->CmdPushConstants(_command_buffer, layout, stageFlags, offset, size, pValues);
}
inline void ResetEvent(VkEvent event, VkPipelineStageFlags stageMask) const {
	_device->CmdResetEvent(_command_buffer, event, stageMask);
}
inline void ResetQueryPool(VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount) const {
	_device->CmdResetQueryPool(_command_buffer, queryPool, firstQuery, queryCount);
}
inline void ResolveImage(VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageResolve* pRegions) const {
	_device->CmdResolveImage(_command_buffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions);
}
inline void SetBlendConstants(const float blendConstants[4]) const {
	_device->CmdSetBlendConstants(_command_buffer, blendConstants);
}
inline void SetDepthBias(float depthBiasConstantFactor, float depthBiasClamp, float depthBiasSlopeFactor) const {
	_device->CmdSetDepthBias(_command_buffer, depthBiasConstantFactor, depthBiasClamp, depthBiasSlopeFactor);
}
inline void SetDepthBounds(float minDepthBounds, float maxDepthBounds) const {
	_device->CmdSetDepthBounds(_command_buffer, minDepthBounds, maxDepthBounds);
}
inline void SetEvent(VkEvent event, VkPipelineStageFlags stageMask) const {
	_device->CmdSetEvent(_command_buffer, event, stageMask);
}
inline void SetLineWidth(float lineWidth) const {
	_device->CmdSetLineWidth(_command_buffer, lineWidth);
}
inline void SetScissor(uint32_t firstScissor, uint32_t scissorCount, const VkRect2D* pScissors) const {
	_device->CmdSetScissor(_command_buffer, firstScissor, scissorCount, pScissors);
}
inline void SetStencilCompareMask(VkStencilFaceFlags faceMask, uint32_t compareMask) const {
	_device->CmdSetStencilCompareMask(_command_buffer, faceMask, compareMask);
}
inline void SetStencilReference(VkStencilFaceFlags faceMask, uint32_t reference) const {
	_device->CmdSetStencilReference(_command_buffer, faceMask, reference);
}
inline void SetStencilWriteMask(VkStencilFaceFlags faceMask, uint32_t writeMask) const {
	_device->CmdSetStencilWriteMask(_command_buffer, faceMask, writeMask);
}
inline void SetViewport(uint32_t firstViewport, uint32_t viewportCount, const VkViewport* pViewports) const {
	_device->CmdSetViewport(_command_buffer, firstViewport, viewportCount, pViewports);
}
inline void UpdateBuffer(VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize dataSize, const void* pData) const {
	_device->CmdUpdateBuffer(_command_buffer, dstBuffer, dstOffset, dataSize, pData);
}
inline void WaitEvents(uint32_t eventCount, const VkEvent* pEvents, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers, uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier* pImageMemoryBarriers) const {
	_device->CmdWaitEvents(_command_buffer, eventCount, pEvents, srcStageMask, dstStageMask, memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
}
inline void WriteTimestamp(VkPipelineStageFlagBits pipelineStage, VkQueryPool queryPool, uint32_t query) const {
	_device->CmdWriteTimestamp(_command_buffer, pipelineStage, queryPool, query);
}
#endif /* defined(VK_VERSION_1_0) */
#if defined(VK_VERSION_1_1)
inline void DispatchBase(uint32_t baseGroupX, uint32_t baseGroupY, uint32_t baseGroupZ, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) const {
	_device->CmdDispatchBase(_command_buffer, baseGroupX, baseGroupY, baseGroupZ, groupCountX, groupCountY, groupCountZ);
}
inline void SetDeviceMask(uint32_t deviceMask) const {
	_device->CmdSetDeviceMask(_command_buffer, deviceMask);
}
#endif /* defined(VK_VERSION_1_1) */
#if defined(VK_VERSION_1_2)
inline void BeginRenderPass2(const VkRenderPassBeginInfo*      pRenderPassBegin, const VkSubpassBeginInfo*      pSubpassBeginInfo) const {
	_device->CmdBeginRenderPass2(_command_buffer, pRenderPassBegin, pSubpassBeginInfo);
}
inline void DrawIndexedIndirectCount(VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride) const {
	_device->CmdDrawIndexedIndirectCount(_command_buffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
}
inline void DrawIndirectCount(VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride) const {
	_device->CmdDrawIndirectCount(_command_buffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
}
inline void EndRenderPass2(const VkSubpassEndInfo*        pSubpassEndInfo) const {
	_device->CmdEndRenderPass2(_command_buffer, pSubpassEndInfo);
}
inline void NextSubpass2(const VkSubpassBeginInfo*      pSubpassBeginInfo, const VkSubpassEndInfo*        pSubpassEndInfo) const {
	_device->CmdNextSubpass2(_command_buffer, pSubpassBeginInfo, pSubpassEndInfo);
}
#endif /* defined(VK_VERSION_1_2) */
#if defined(VK_VERSION_1_3)
inline void BeginRendering(const VkRenderingInfo*                              pRenderingInfo) const {
	_device->CmdBeginRendering(_command_buffer, pRenderingInfo);
}
inline void BindVertexBuffers2(uint32_t firstBinding, uint32_t bindingCount, const VkBuffer* pBuffers, const VkDeviceSize* pOffsets, const VkDeviceSize* pSizes, const VkDeviceSize* pStrides) const {
	_device->CmdBindVertexBuffers2(_command_buffer, firstBinding, bindingCount, pBuffers, pOffsets, pSizes, pStrides);
}
inline void BlitImage2(const VkBlitImageInfo2* pBlitImageInfo) const {
	_device->CmdBlitImage2(_command_buffer, pBlitImageInfo);
}
inline void CopyBuffer2(const VkCopyBufferInfo2* pCopyBufferInfo) const {
	_device->CmdCopyBuffer2(_command_buffer, pCopyBufferInfo);
}
inline void CopyBufferToImage2(const VkCopyBufferToImageInfo2* pCopyBufferToImageInfo) const {
	_device->CmdCopyBufferToImage2(_command_buffer, pCopyBufferToImageInfo);
}
inline void CopyImage2(const VkCopyImageInfo2* pCopyImageInfo) const {
	_device->CmdCopyImage2(_command_buffer, pCopyImageInfo);
}
inline void CopyImageToBuffer2(const VkCopyImageToBufferInfo2* pCopyImageToBufferInfo) const {
	_device->CmdCopyImageToBuffer2(_command_buffer, pCopyImageToBufferInfo);
}
inline void EndRendering() const {
	_device->CmdEndRendering(_command_buffer);
}
inline void PipelineBarrier2(const VkDependencyInfo*                             pDependencyInfo) const {
	_device->CmdPipelineBarrier2(_command_buffer, pDependencyInfo);
}
inline void ResetEvent2(VkEvent                                             event, VkPipelineStageFlags2               stageMask) const {
	_device->CmdResetEvent2(_command_buffer, event, stageMask);
}
inline void ResolveImage2(const VkResolveImageInfo2* pResolveImageInfo) const {
	_device->CmdResolveImage2(_command_buffer, pResolveImageInfo);
}
inline void SetCullMode(VkCullModeFlags cullMode) const {
	_device->CmdSetCullMode(_command_buffer, cullMode);
}
inline void SetDepthBiasEnable(VkBool32 depthBiasEnable) const {
	_device->CmdSetDepthBiasEnable(_command_buffer, depthBiasEnable);
}
inline void SetDepthBoundsTestEnable(VkBool32 depthBoundsTestEnable) const {
	_device->CmdSetDepthBoundsTestEnable(_command_buffer, depthBoundsTestEnable);
}
inline void SetDepthCompareOp(VkCompareOp depthCompareOp) const {
	_device->CmdSetDepthCompareOp(_command_buffer, depthCompareOp);
}
inline void SetDepthTestEnable(VkBool32 depthTestEnable) const {
	_device->CmdSetDepthTestEnable(_command_buffer, depthTestEnable);
}
inline void SetDepthWriteEnable(VkBool32 depthWriteEnable) const {
	_device->CmdSetDepthWriteEnable(_command_buffer, depthWriteEnable);
}
inline void SetEvent2(VkEvent                                             event, const VkDependencyInfo*                             pDependencyInfo) const {
	_device->CmdSetEvent2(_command_buffer, event, pDependencyInfo);
}
inline void SetFrontFace(VkFrontFace frontFace) const {
	_device->CmdSetFrontFace(_command_buffer, frontFace);
}
inline void SetPrimitiveRestartEnable(VkBool32 primitiveRestartEnable) const {
	_device->CmdSetPrimitiveRestartEnable(_command_buffer, primitiveRestartEnable);
}
inline void SetPrimitiveTopology(VkPrimitiveTopology primitiveTopology) const {
	_device->CmdSetPrimitiveTopology(_command_buffer, primitiveTopology);
}
inline void SetRasterizerDiscardEnable(VkBool32 rasterizerDiscardEnable) const {
	_device->CmdSetRasterizerDiscardEnable(_command_buffer, rasterizerDiscardEnable);
}
inline void SetScissorWithCount(uint32_t scissorCount, const VkRect2D* pScissors) const {
	_device->CmdSetScissorWithCount(_command_buffer, scissorCount, pScissors);
}
inline void SetStencilOp(VkStencilFaceFlags faceMask, VkStencilOp failOp, VkStencilOp passOp, VkStencilOp depthFailOp, VkCompareOp compareOp) const {
	_device->CmdSetStencilOp(_command_buffer, faceMask, failOp, passOp, depthFailOp, compareOp);
}
inline void SetStencilTestEnable(VkBool32 stencilTestEnable) const {
	_device->CmdSetStencilTestEnable(_command_buffer, stencilTestEnable);
}
inline void SetViewportWithCount(uint32_t viewportCount, const VkViewport* pViewports) const {
	_device->CmdSetViewportWithCount(_command_buffer, viewportCount, pViewports);
}
inline void WaitEvents2(uint32_t                                            eventCount, const VkEvent*                     pEvents, const VkDependencyInfo*            pDependencyInfos) const {
	_device->CmdWaitEvents2(_command_buffer, eventCount, pEvents, pDependencyInfos);
}
inline void WriteTimestamp2(VkPipelineStageFlags2               stage, VkQueryPool                                         queryPool, uint32_t                                            query) const {
	_device->CmdWriteTimestamp2(_command_buffer, stage, queryPool, query);
}
#endif /* defined(VK_VERSION_1_3) */
#if defined(VK_AMD_buffer_marker)
inline void WriteBufferMarkerAMD(VkPipelineStageFlagBits pipelineStage, VkBuffer dstBuffer, VkDeviceSize dstOffset, uint32_t marker) const {
	_device->CmdWriteBufferMarkerAMD(_command_buffer, pipelineStage, dstBuffer, dstOffset, marker);
}
#endif /* defined(VK_AMD_buffer_marker) */
#if defined(VK_AMD_draw_indirect_count)
inline void DrawIndexedIndirectCountAMD(VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride) const {
	_device->CmdDrawIndexedIndirectCountAMD(_command_buffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
}
inline void DrawIndirectCountAMD(VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride) const {
	_device->CmdDrawIndirectCountAMD(_command_buffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
}
#endif /* defined(VK_AMD_draw_indirect_count) */
#if defined(VK_EXT_color_write_enable)
inline void SetColorWriteEnableEXT(uint32_t                                attachmentCount, const VkBool32*   pColorWriteEnables) const {
	_device->CmdSetColorWriteEnableEXT(_command_buffer, attachmentCount, pColorWriteEnables);
}
#endif /* defined(VK_EXT_color_write_enable) */
#if defined(VK_EXT_conditional_rendering)
inline void BeginConditionalRenderingEXT(const VkConditionalRenderingBeginInfoEXT* pConditionalRenderingBegin) const {
	_device->CmdBeginConditionalRenderingEXT(_command_buffer, pConditionalRenderingBegin);
}
inline void EndConditionalRenderingEXT() const {
	_device->CmdEndConditionalRenderingEXT(_command_buffer);
}
#endif /* defined(VK_EXT_conditional_rendering) */
#if defined(VK_EXT_debug_marker)
inline void DebugMarkerBeginEXT(const VkDebugMarkerMarkerInfoEXT* pMarkerInfo) const {
	_device->CmdDebugMarkerBeginEXT(_command_buffer, pMarkerInfo);
}
inline void DebugMarkerEndEXT() const {
	_device->CmdDebugMarkerEndEXT(_command_buffer);
}
inline void DebugMarkerInsertEXT(const VkDebugMarkerMarkerInfoEXT* pMarkerInfo) const {
	_device->CmdDebugMarkerInsertEXT(_command_buffer, pMarkerInfo);
}
#endif /* defined(VK_EXT_debug_marker) */
#if defined(VK_EXT_discard_rectangles)
inline void SetDiscardRectangleEXT(uint32_t firstDiscardRectangle, uint32_t discardRectangleCount, const VkRect2D* pDiscardRectangles) const {
	_device->CmdSetDiscardRectangleEXT(_command_buffer, firstDiscardRectangle, discardRectangleCount, pDiscardRectangles);
}
#endif /* defined(VK_EXT_discard_rectangles) */
#if defined(VK_EXT_extended_dynamic_state)
inline void BindVertexBuffers2EXT(uint32_t firstBinding, uint32_t bindingCount, const VkBuffer* pBuffers, const VkDeviceSize* pOffsets, const VkDeviceSize* pSizes, const VkDeviceSize* pStrides) const {
	_device->CmdBindVertexBuffers2EXT(_command_buffer, firstBinding, bindingCount, pBuffers, pOffsets, pSizes, pStrides);
}
inline void SetCullModeEXT(VkCullModeFlags cullMode) const {
	_device->CmdSetCullModeEXT(_command_buffer, cullMode);
}
inline void SetDepthBoundsTestEnableEXT(VkBool32 depthBoundsTestEnable) const {
	_device->CmdSetDepthBoundsTestEnableEXT(_command_buffer, depthBoundsTestEnable);
}
inline void SetDepthCompareOpEXT(VkCompareOp depthCompareOp) const {
	_device->CmdSetDepthCompareOpEXT(_command_buffer, depthCompareOp);
}
inline void SetDepthTestEnableEXT(VkBool32 depthTestEnable) const {
	_device->CmdSetDepthTestEnableEXT(_command_buffer, depthTestEnable);
}
inline void SetDepthWriteEnableEXT(VkBool32 depthWriteEnable) const {
	_device->CmdSetDepthWriteEnableEXT(_command_buffer, depthWriteEnable);
}
inline void SetFrontFaceEXT(VkFrontFace frontFace) const {
	_device->CmdSetFrontFaceEXT(_command_buffer, frontFace);
}
inline void SetPrimitiveTopologyEXT(VkPrimitiveTopology primitiveTopology) const {
	_device->CmdSetPrimitiveTopologyEXT(_command_buffer, primitiveTopology);
}
inline void SetScissorWithCountEXT(uint32_t scissorCount, const VkRect2D* pScissors) const {
	_device->CmdSetScissorWithCountEXT(_command_buffer, scissorCount, pScissors);
}
inline void SetStencilOpEXT(VkStencilFaceFlags faceMask, VkStencilOp failOp, VkStencilOp passOp, VkStencilOp depthFailOp, VkCompareOp compareOp) const {
	_device->CmdSetStencilOpEXT(_command_buffer, faceMask, failOp, passOp, depthFailOp, compareOp);
}
inline void SetStencilTestEnableEXT(VkBool32 stencilTestEnable) const {
	_device->CmdSetStencilTestEnableEXT(_command_buffer, stencilTestEnable);
}
inline void SetViewportWithCountEXT(uint32_t viewportCount, const VkViewport* pViewports) const {
	_device->CmdSetViewportWithCountEXT(_command_buffer, viewportCount, pViewports);
}
#endif /* defined(VK_EXT_extended_dynamic_state) */
#if defined(VK_EXT_extended_dynamic_state2)
inline void SetDepthBiasEnableEXT(VkBool32 depthBiasEnable) const {
	_device->CmdSetDepthBiasEnableEXT(_command_buffer, depthBiasEnable);
}
inline void SetLogicOpEXT(VkLogicOp logicOp) const {
	_device->CmdSetLogicOpEXT(_command_buffer, logicOp);
}
inline void SetPatchControlPointsEXT(uint32_t patchControlPoints) const {
	_device->CmdSetPatchControlPointsEXT(_command_buffer, patchControlPoints);
}
inline void SetPrimitiveRestartEnableEXT(VkBool32 primitiveRestartEnable) const {
	_device->CmdSetPrimitiveRestartEnableEXT(_command_buffer, primitiveRestartEnable);
}
inline void SetRasterizerDiscardEnableEXT(VkBool32 rasterizerDiscardEnable) const {
	_device->CmdSetRasterizerDiscardEnableEXT(_command_buffer, rasterizerDiscardEnable);
}
#endif /* defined(VK_EXT_extended_dynamic_state2) */
#if defined(VK_EXT_line_rasterization)
inline void SetLineStippleEXT(uint32_t lineStippleFactor, uint16_t lineStipplePattern) const {
	_device->CmdSetLineStippleEXT(_command_buffer, lineStippleFactor, lineStipplePattern);
}
#endif /* defined(VK_EXT_line_rasterization) */
#if defined(VK_EXT_multi_draw)
inline void DrawMultiEXT(uint32_t drawCount, const VkMultiDrawInfoEXT* pVertexInfo, uint32_t instanceCount, uint32_t firstInstance, uint32_t stride) const {
	_device->CmdDrawMultiEXT(_command_buffer, drawCount, pVertexInfo, instanceCount, firstInstance, stride);
}
inline void DrawMultiIndexedEXT(uint32_t drawCount, const VkMultiDrawIndexedInfoEXT* pIndexInfo, uint32_t instanceCount, uint32_t firstInstance, uint32_t stride, const int32_t* pVertexOffset) const {
	_device->CmdDrawMultiIndexedEXT(_command_buffer, drawCount, pIndexInfo, instanceCount, firstInstance, stride, pVertexOffset);
}
#endif /* defined(VK_EXT_multi_draw) */
#if defined(VK_EXT_sample_locations)
inline void SetSampleLocationsEXT(const VkSampleLocationsInfoEXT* pSampleLocationsInfo) const {
	_device->CmdSetSampleLocationsEXT(_command_buffer, pSampleLocationsInfo);
}
#endif /* defined(VK_EXT_sample_locations) */
#if defined(VK_EXT_transform_feedback)
inline void BeginQueryIndexedEXT(VkQueryPool queryPool, uint32_t query, VkQueryControlFlags flags, uint32_t index) const {
	_device->CmdBeginQueryIndexedEXT(_command_buffer, queryPool, query, flags, index);
}
inline void BeginTransformFeedbackEXT(uint32_t firstCounterBuffer, uint32_t counterBufferCount, const VkBuffer* pCounterBuffers, const VkDeviceSize* pCounterBufferOffsets) const {
	_device->CmdBeginTransformFeedbackEXT(_command_buffer, firstCounterBuffer, counterBufferCount, pCounterBuffers, pCounterBufferOffsets);
}
inline void BindTransformFeedbackBuffersEXT(uint32_t firstBinding, uint32_t bindingCount, const VkBuffer* pBuffers, const VkDeviceSize* pOffsets, const VkDeviceSize* pSizes) const {
	_device->CmdBindTransformFeedbackBuffersEXT(_command_buffer, firstBinding, bindingCount, pBuffers, pOffsets, pSizes);
}
inline void DrawIndirectByteCountEXT(uint32_t instanceCount, uint32_t firstInstance, VkBuffer counterBuffer, VkDeviceSize counterBufferOffset, uint32_t counterOffset, uint32_t vertexStride) const {
	_device->CmdDrawIndirectByteCountEXT(_command_buffer, instanceCount, firstInstance, counterBuffer, counterBufferOffset, counterOffset, vertexStride);
}
inline void EndQueryIndexedEXT(VkQueryPool queryPool, uint32_t query, uint32_t index) const {
	_device->CmdEndQueryIndexedEXT(_command_buffer, queryPool, query, index);
}
inline void EndTransformFeedbackEXT(uint32_t firstCounterBuffer, uint32_t counterBufferCount, const VkBuffer* pCounterBuffers, const VkDeviceSize* pCounterBufferOffsets) const {
	_device->CmdEndTransformFeedbackEXT(_command_buffer, firstCounterBuffer, counterBufferCount, pCounterBuffers, pCounterBufferOffsets);
}
#endif /* defined(VK_EXT_transform_feedback) */
#if defined(VK_EXT_vertex_input_dynamic_state)
inline void SetVertexInputEXT(uint32_t vertexBindingDescriptionCount, const VkVertexInputBindingDescription2EXT* pVertexBindingDescriptions, uint32_t vertexAttributeDescriptionCount, const VkVertexInputAttributeDescription2EXT* pVertexAttributeDescriptions) const {
	_device->CmdSetVertexInputEXT(_command_buffer, vertexBindingDescriptionCount, pVertexBindingDescriptions, vertexAttributeDescriptionCount, pVertexAttributeDescriptions);
}
#endif /* defined(VK_EXT_vertex_input_dynamic_state) */
#if defined(VK_HUAWEI_invocation_mask)
inline void BindInvocationMaskHUAWEI(VkImageView imageView, VkImageLayout imageLayout) const {
	_device->CmdBindInvocationMaskHUAWEI(_command_buffer, imageView, imageLayout);
}
#endif /* defined(VK_HUAWEI_invocation_mask) */
#if defined(VK_HUAWEI_subpass_shading)
inline void SubpassShadingHUAWEI() const {
	_device->CmdSubpassShadingHUAWEI(_command_buffer);
}
#endif /* defined(VK_HUAWEI_subpass_shading) */
#if defined(VK_INTEL_performance_query)
inline VkResult SetPerformanceMarkerINTEL(const VkPerformanceMarkerInfoINTEL* pMarkerInfo) const {
	return _device->CmdSetPerformanceMarkerINTEL(_command_buffer, pMarkerInfo);
}
inline VkResult SetPerformanceOverrideINTEL(const VkPerformanceOverrideInfoINTEL* pOverrideInfo) const {
	return _device->CmdSetPerformanceOverrideINTEL(_command_buffer, pOverrideInfo);
}
inline VkResult SetPerformanceStreamMarkerINTEL(const VkPerformanceStreamMarkerInfoINTEL* pMarkerInfo) const {
	return _device->CmdSetPerformanceStreamMarkerINTEL(_command_buffer, pMarkerInfo);
}
#endif /* defined(VK_INTEL_performance_query) */
#if defined(VK_KHR_acceleration_structure)
inline void BuildAccelerationStructuresIndirectKHR(uint32_t                                           infoCount, const VkAccelerationStructureBuildGeometryInfoKHR* pInfos, const VkDeviceAddress*             pIndirectDeviceAddresses, const uint32_t*                    pIndirectStrides, const uint32_t* const*             ppMaxPrimitiveCounts) const {
	_device->CmdBuildAccelerationStructuresIndirectKHR(_command_buffer, infoCount, pInfos, pIndirectDeviceAddresses, pIndirectStrides, ppMaxPrimitiveCounts);
}
inline void BuildAccelerationStructuresKHR(uint32_t infoCount, const VkAccelerationStructureBuildGeometryInfoKHR* pInfos, const VkAccelerationStructureBuildRangeInfoKHR* const* ppBuildRangeInfos) const {
	_device->CmdBuildAccelerationStructuresKHR(_command_buffer, infoCount, pInfos, ppBuildRangeInfos);
}
inline void CopyAccelerationStructureKHR(const VkCopyAccelerationStructureInfoKHR* pInfo) const {
	_device->CmdCopyAccelerationStructureKHR(_command_buffer, pInfo);
}
inline void CopyAccelerationStructureToMemoryKHR(const VkCopyAccelerationStructureToMemoryInfoKHR* pInfo) const {
	_device->CmdCopyAccelerationStructureToMemoryKHR(_command_buffer, pInfo);
}
inline void CopyMemoryToAccelerationStructureKHR(const VkCopyMemoryToAccelerationStructureInfoKHR* pInfo) const {
	_device->CmdCopyMemoryToAccelerationStructureKHR(_command_buffer, pInfo);
}
inline void WriteAccelerationStructuresPropertiesKHR(uint32_t accelerationStructureCount, const VkAccelerationStructureKHR* pAccelerationStructures, VkQueryType queryType, VkQueryPool queryPool, uint32_t firstQuery) const {
	_device->CmdWriteAccelerationStructuresPropertiesKHR(_command_buffer, accelerationStructureCount, pAccelerationStructures, queryType, queryPool, firstQuery);
}
#endif /* defined(VK_KHR_acceleration_structure) */
#if defined(VK_KHR_copy_commands2)
inline void BlitImage2KHR(const VkBlitImageInfo2* pBlitImageInfo) const {
	_device->CmdBlitImage2KHR(_command_buffer, pBlitImageInfo);
}
inline void CopyBuffer2KHR(const VkCopyBufferInfo2* pCopyBufferInfo) const {
	_device->CmdCopyBuffer2KHR(_command_buffer, pCopyBufferInfo);
}
inline void CopyBufferToImage2KHR(const VkCopyBufferToImageInfo2* pCopyBufferToImageInfo) const {
	_device->CmdCopyBufferToImage2KHR(_command_buffer, pCopyBufferToImageInfo);
}
inline void CopyImage2KHR(const VkCopyImageInfo2* pCopyImageInfo) const {
	_device->CmdCopyImage2KHR(_command_buffer, pCopyImageInfo);
}
inline void CopyImageToBuffer2KHR(const VkCopyImageToBufferInfo2* pCopyImageToBufferInfo) const {
	_device->CmdCopyImageToBuffer2KHR(_command_buffer, pCopyImageToBufferInfo);
}
inline void ResolveImage2KHR(const VkResolveImageInfo2* pResolveImageInfo) const {
	_device->CmdResolveImage2KHR(_command_buffer, pResolveImageInfo);
}
#endif /* defined(VK_KHR_copy_commands2) */
#if defined(VK_KHR_create_renderpass2)
inline void BeginRenderPass2KHR(const VkRenderPassBeginInfo*      pRenderPassBegin, const VkSubpassBeginInfo*      pSubpassBeginInfo) const {
	_device->CmdBeginRenderPass2KHR(_command_buffer, pRenderPassBegin, pSubpassBeginInfo);
}
inline void EndRenderPass2KHR(const VkSubpassEndInfo*        pSubpassEndInfo) const {
	_device->CmdEndRenderPass2KHR(_command_buffer, pSubpassEndInfo);
}
inline void NextSubpass2KHR(const VkSubpassBeginInfo*      pSubpassBeginInfo, const VkSubpassEndInfo*        pSubpassEndInfo) const {
	_device->CmdNextSubpass2KHR(_command_buffer, pSubpassBeginInfo, pSubpassEndInfo);
}
#endif /* defined(VK_KHR_create_renderpass2) */
#if defined(VK_KHR_device_group)
inline void DispatchBaseKHR(uint32_t baseGroupX, uint32_t baseGroupY, uint32_t baseGroupZ, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) const {
	_device->CmdDispatchBaseKHR(_command_buffer, baseGroupX, baseGroupY, baseGroupZ, groupCountX, groupCountY, groupCountZ);
}
inline void SetDeviceMaskKHR(uint32_t deviceMask) const {
	_device->CmdSetDeviceMaskKHR(_command_buffer, deviceMask);
}
#endif /* defined(VK_KHR_device_group) */
#if defined(VK_KHR_draw_indirect_count)
inline void DrawIndexedIndirectCountKHR(VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride) const {
	_device->CmdDrawIndexedIndirectCountKHR(_command_buffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
}
inline void DrawIndirectCountKHR(VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride) const {
	_device->CmdDrawIndirectCountKHR(_command_buffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
}
#endif /* defined(VK_KHR_draw_indirect_count) */
#if defined(VK_KHR_dynamic_rendering)
inline void BeginRenderingKHR(const VkRenderingInfo*                              pRenderingInfo) const {
	_device->CmdBeginRenderingKHR(_command_buffer, pRenderingInfo);
}
inline void EndRenderingKHR() const {
	_device->CmdEndRenderingKHR(_command_buffer);
}
#endif /* defined(VK_KHR_dynamic_rendering) */
#if defined(VK_KHR_fragment_shading_rate)
inline void SetFragmentShadingRateKHR(const VkExtent2D*                           pFragmentSize, const VkFragmentShadingRateCombinerOpKHR    combinerOps[2]) const {
	_device->CmdSetFragmentShadingRateKHR(_command_buffer, pFragmentSize, combinerOps);
}
#endif /* defined(VK_KHR_fragment_shading_rate) */
#if defined(VK_KHR_push_descriptor)
inline void PushDescriptorSetKHR(VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout, uint32_t set, uint32_t descriptorWriteCount, const VkWriteDescriptorSet* pDescriptorWrites) const {
	_device->CmdPushDescriptorSetKHR(_command_buffer, pipelineBindPoint, layout, set, descriptorWriteCount, pDescriptorWrites);
}
#endif /* defined(VK_KHR_push_descriptor) */
#if defined(VK_KHR_ray_tracing_maintenance1) && defined(VK_KHR_ray_tracing_pipeline)
inline void TraceRaysIndirect2KHR(VkDeviceAddress indirectDeviceAddress) const {
	_device->CmdTraceRaysIndirect2KHR(_command_buffer, indirectDeviceAddress);
}
#endif /* defined(VK_KHR_ray_tracing_maintenance1) && defined(VK_KHR_ray_tracing_pipeline) */
#if defined(VK_KHR_ray_tracing_pipeline)
inline void SetRayTracingPipelineStackSizeKHR(uint32_t pipelineStackSize) const {
	_device->CmdSetRayTracingPipelineStackSizeKHR(_command_buffer, pipelineStackSize);
}
inline void TraceRaysIndirectKHR(const VkStridedDeviceAddressRegionKHR* pRaygenShaderBindingTable, const VkStridedDeviceAddressRegionKHR* pMissShaderBindingTable, const VkStridedDeviceAddressRegionKHR* pHitShaderBindingTable, const VkStridedDeviceAddressRegionKHR* pCallableShaderBindingTable, VkDeviceAddress indirectDeviceAddress) const {
	_device->CmdTraceRaysIndirectKHR(_command_buffer, pRaygenShaderBindingTable, pMissShaderBindingTable, pHitShaderBindingTable, pCallableShaderBindingTable, indirectDeviceAddress);
}
inline void TraceRaysKHR(const VkStridedDeviceAddressRegionKHR* pRaygenShaderBindingTable, const VkStridedDeviceAddressRegionKHR* pMissShaderBindingTable, const VkStridedDeviceAddressRegionKHR* pHitShaderBindingTable, const VkStridedDeviceAddressRegionKHR* pCallableShaderBindingTable, uint32_t width, uint32_t height, uint32_t depth) const {
	_device->CmdTraceRaysKHR(_command_buffer, pRaygenShaderBindingTable, pMissShaderBindingTable, pHitShaderBindingTable, pCallableShaderBindingTable, width, height, depth);
}
#endif /* defined(VK_KHR_ray_tracing_pipeline) */
#if defined(VK_KHR_synchronization2)
inline void PipelineBarrier2KHR(const VkDependencyInfo*                             pDependencyInfo) const {
	_device->CmdPipelineBarrier2KHR(_command_buffer, pDependencyInfo);
}
inline void ResetEvent2KHR(VkEvent                                             event, VkPipelineStageFlags2               stageMask) const {
	_device->CmdResetEvent2KHR(_command_buffer, event, stageMask);
}
inline void SetEvent2KHR(VkEvent                                             event, const VkDependencyInfo*                             pDependencyInfo) const {
	_device->CmdSetEvent2KHR(_command_buffer, event, pDependencyInfo);
}
inline void WaitEvents2KHR(uint32_t                                            eventCount, const VkEvent*                     pEvents, const VkDependencyInfo*            pDependencyInfos) const {
	_device->CmdWaitEvents2KHR(_command_buffer, eventCount, pEvents, pDependencyInfos);
}
inline void WriteTimestamp2KHR(VkPipelineStageFlags2               stage, VkQueryPool                                         queryPool, uint32_t                                            query) const {
	_device->CmdWriteTimestamp2KHR(_command_buffer, stage, queryPool, query);
}
#endif /* defined(VK_KHR_synchronization2) */
#if defined(VK_KHR_synchronization2) && defined(VK_AMD_buffer_marker)
inline void WriteBufferMarker2AMD(VkPipelineStageFlags2               stage, VkBuffer                                            dstBuffer, VkDeviceSize                                        dstOffset, uint32_t                                            marker) const {
	_device->CmdWriteBufferMarker2AMD(_command_buffer, stage, dstBuffer, dstOffset, marker);
}
#endif /* defined(VK_KHR_synchronization2) && defined(VK_AMD_buffer_marker) */
#if defined(VK_KHR_video_decode_queue)
inline void DecodeVideoKHR(const VkVideoDecodeInfoKHR* pFrameInfo) const {
	_device->CmdDecodeVideoKHR(_command_buffer, pFrameInfo);
}
#endif /* defined(VK_KHR_video_decode_queue) */
#if defined(VK_KHR_video_encode_queue)
inline void EncodeVideoKHR(const VkVideoEncodeInfoKHR* pEncodeInfo) const {
	_device->CmdEncodeVideoKHR(_command_buffer, pEncodeInfo);
}
#endif /* defined(VK_KHR_video_encode_queue) */
#if defined(VK_KHR_video_queue)
inline void BeginVideoCodingKHR(const VkVideoBeginCodingInfoKHR* pBeginInfo) const {
	_device->CmdBeginVideoCodingKHR(_command_buffer, pBeginInfo);
}
inline void ControlVideoCodingKHR(const VkVideoCodingControlInfoKHR* pCodingControlInfo) const {
	_device->CmdControlVideoCodingKHR(_command_buffer, pCodingControlInfo);
}
inline void EndVideoCodingKHR(const VkVideoEndCodingInfoKHR* pEndCodingInfo) const {
	_device->CmdEndVideoCodingKHR(_command_buffer, pEndCodingInfo);
}
#endif /* defined(VK_KHR_video_queue) */
#if defined(VK_NVX_binary_import)
inline void CuLaunchKernelNVX(const VkCuLaunchInfoNVX* pLaunchInfo) const {
	_device->CmdCuLaunchKernelNVX(_command_buffer, pLaunchInfo);
}
#endif /* defined(VK_NVX_binary_import) */
#if defined(VK_NV_clip_space_w_scaling)
inline void SetViewportWScalingNV(uint32_t firstViewport, uint32_t viewportCount, const VkViewportWScalingNV* pViewportWScalings) const {
	_device->CmdSetViewportWScalingNV(_command_buffer, firstViewport, viewportCount, pViewportWScalings);
}
#endif /* defined(VK_NV_clip_space_w_scaling) */
#if defined(VK_NV_device_diagnostic_checkpoints)
inline void SetCheckpointNV(const void* pCheckpointMarker) const {
	_device->CmdSetCheckpointNV(_command_buffer, pCheckpointMarker);
}
#endif /* defined(VK_NV_device_diagnostic_checkpoints) */
#if defined(VK_NV_device_generated_commands)
inline void BindPipelineShaderGroupNV(VkPipelineBindPoint pipelineBindPoint, VkPipeline pipeline, uint32_t groupIndex) const {
	_device->CmdBindPipelineShaderGroupNV(_command_buffer, pipelineBindPoint, pipeline, groupIndex);
}
inline void ExecuteGeneratedCommandsNV(VkBool32 isPreprocessed, const VkGeneratedCommandsInfoNV* pGeneratedCommandsInfo) const {
	_device->CmdExecuteGeneratedCommandsNV(_command_buffer, isPreprocessed, pGeneratedCommandsInfo);
}
inline void PreprocessGeneratedCommandsNV(const VkGeneratedCommandsInfoNV* pGeneratedCommandsInfo) const {
	_device->CmdPreprocessGeneratedCommandsNV(_command_buffer, pGeneratedCommandsInfo);
}
#endif /* defined(VK_NV_device_generated_commands) */
#if defined(VK_NV_fragment_shading_rate_enums)
inline void SetFragmentShadingRateEnumNV(VkFragmentShadingRateNV                     shadingRate, const VkFragmentShadingRateCombinerOpKHR    combinerOps[2]) const {
	_device->CmdSetFragmentShadingRateEnumNV(_command_buffer, shadingRate, combinerOps);
}
#endif /* defined(VK_NV_fragment_shading_rate_enums) */
#if defined(VK_NV_mesh_shader)
inline void DrawMeshTasksIndirectCountNV(VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride) const {
	_device->CmdDrawMeshTasksIndirectCountNV(_command_buffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
}
inline void DrawMeshTasksIndirectNV(VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount, uint32_t stride) const {
	_device->CmdDrawMeshTasksIndirectNV(_command_buffer, buffer, offset, drawCount, stride);
}
inline void DrawMeshTasksNV(uint32_t taskCount, uint32_t firstTask) const {
	_device->CmdDrawMeshTasksNV(_command_buffer, taskCount, firstTask);
}
#endif /* defined(VK_NV_mesh_shader) */
#if defined(VK_NV_ray_tracing)
inline void BuildAccelerationStructureNV(const VkAccelerationStructureInfoNV* pInfo, VkBuffer instanceData, VkDeviceSize instanceOffset, VkBool32 update, VkAccelerationStructureNV dst, VkAccelerationStructureNV src, VkBuffer scratch, VkDeviceSize scratchOffset) const {
	_device->CmdBuildAccelerationStructureNV(_command_buffer, pInfo, instanceData, instanceOffset, update, dst, src, scratch, scratchOffset);
}
inline void CopyAccelerationStructureNV(VkAccelerationStructureNV dst, VkAccelerationStructureNV src, VkCopyAccelerationStructureModeKHR mode) const {
	_device->CmdCopyAccelerationStructureNV(_command_buffer, dst, src, mode);
}
inline void TraceRaysNV(VkBuffer raygenShaderBindingTableBuffer, VkDeviceSize raygenShaderBindingOffset, VkBuffer missShaderBindingTableBuffer, VkDeviceSize missShaderBindingOffset, VkDeviceSize missShaderBindingStride, VkBuffer hitShaderBindingTableBuffer, VkDeviceSize hitShaderBindingOffset, VkDeviceSize hitShaderBindingStride, VkBuffer callableShaderBindingTableBuffer, VkDeviceSize callableShaderBindingOffset, VkDeviceSize callableShaderBindingStride, uint32_t width, uint32_t height, uint32_t depth) const {
	_device->CmdTraceRaysNV(_command_buffer, raygenShaderBindingTableBuffer, raygenShaderBindingOffset, missShaderBindingTableBuffer, missShaderBindingOffset, missShaderBindingStride, hitShaderBindingTableBuffer, hitShaderBindingOffset, hitShaderBindingStride, callableShaderBindingTableBuffer, callableShaderBindingOffset, callableShaderBindingStride, width, height, depth);
}
inline void WriteAccelerationStructuresPropertiesNV(uint32_t accelerationStructureCount, const VkAccelerationStructureNV* pAccelerationStructures, VkQueryType queryType, VkQueryPool queryPool, uint32_t firstQuery) const {
	_device->CmdWriteAccelerationStructuresPropertiesNV(_command_buffer, accelerationStructureCount, pAccelerationStructures, queryType, queryPool, firstQuery);
}
#endif /* defined(VK_NV_ray_tracing) */
#if defined(VK_NV_scissor_exclusive)
inline void SetExclusiveScissorNV(uint32_t firstExclusiveScissor, uint32_t exclusiveScissorCount, const VkRect2D* pExclusiveScissors) const {
	_device->CmdSetExclusiveScissorNV(_command_buffer, firstExclusiveScissor, exclusiveScissorCount, pExclusiveScissors);
}
#endif /* defined(VK_NV_scissor_exclusive) */
#if defined(VK_NV_shading_rate_image)
inline void BindShadingRateImageNV(VkImageView imageView, VkImageLayout imageLayout) const {
	_device->CmdBindShadingRateImageNV(_command_buffer, imageView, imageLayout);
}
inline void SetCoarseSampleOrderNV(VkCoarseSampleOrderTypeNV sampleOrderType, uint32_t customSampleOrderCount, const VkCoarseSampleOrderCustomNV* pCustomSampleOrders) const {
	_device->CmdSetCoarseSampleOrderNV(_command_buffer, sampleOrderType, customSampleOrderCount, pCustomSampleOrders);
}
inline void SetViewportShadingRatePaletteNV(uint32_t firstViewport, uint32_t viewportCount, const VkShadingRatePaletteNV* pShadingRatePalettes) const {
	_device->CmdSetViewportShadingRatePaletteNV(_command_buffer, firstViewport, viewportCount, pShadingRatePalettes);
}
#endif /* defined(VK_NV_shading_rate_image) */
#if (defined(VK_KHR_descriptor_update_template) && defined(VK_KHR_push_descriptor)) || (defined(VK_KHR_push_descriptor) && defined(VK_VERSION_1_1)) || (defined(VK_KHR_push_descriptor) && defined(VK_KHR_descriptor_update_template))
inline void PushDescriptorSetWithTemplateKHR(VkDescriptorUpdateTemplate descriptorUpdateTemplate, VkPipelineLayout layout, uint32_t set, const void* pData) const {
	_device->CmdPushDescriptorSetWithTemplateKHR(_command_buffer, descriptorUpdateTemplate, layout, set, pData);
}
#endif /* (defined(VK_KHR_descriptor_update_template) && defined(VK_KHR_push_descriptor)) || (defined(VK_KHR_push_descriptor) && defined(VK_VERSION_1_1)) || (defined(VK_KHR_push_descriptor) && defined(VK_KHR_descriptor_update_template)) */
  /* VOLK_GENERATE_COMMAND_BUFFER_METHOD_HPP */
};

} // namespace VOLK_NAMESPACE

#endif