//
// Created by jello_ on 7/3/2022.
//

#ifndef PLUTONIUM_BUFFERS_H
#define PLUTONIUM_BUFFERS_H

#include <stdlib.h>
#include <string.h>

#include <vulkan/vulkan.h>

#include "Globals.h"
#include "Descriptors.h"

enum MemoryProperties {
    GPU_LOCAL = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
    CPU_VISIBLE = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
    CPU_COHERENT = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
    CPU_CACHED = VK_MEMORY_PROPERTY_HOST_CACHED_BIT,
    LAZY_ALLOCATED = VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT,
    PROTECTED_MEMORY = VK_MEMORY_PROPERTY_PROTECTED_BIT,
};

void InitializeBufferInfo(VkPhysicalDevice physicalDevice);

typedef struct { VkBuffer buffer; VkDeviceMemory memory; } Buffer;
typedef struct { Buffer buffer; VkDescriptorBufferInfo bufferInfo; SingleDescriptor descriptor; } UniformBuffer;

Buffer CreateBuffer(VkDevice device, VkDeviceSize size, U32 queue, VkBufferUsageFlagBits usage, VkMemoryPropertyFlagBits memoryFlags);

Buffer CreateBufferToGPU(VkDevice device, U32 queue, VkDeviceSize size, VkBufferUsageFlagBits usageFlags, void* data);

void UploadDataToBuffer(VkDevice device, Buffer* buffer, VkDeviceSize size, void* data);

UniformBuffer CreateUniformBuffers(VkDevice device, VkDeviceSize size, VkShaderStageFlagBits stage, U32 binding, U32 queue);

void UpdateUniform(VkDevice device, UniformBuffer* buffer, VkDeviceSize size, void* data);

void BindUniform(VkCommandBuffer cmdBuffer, VkPipelineBindPoint bindPoint, VkPipelineLayout pipelineLayout, UniformBuffer buffer);

void destroyBuffer(VkDevice device, Buffer* buffer);








#endif //PLUTONIUM_BUFFERS_H
