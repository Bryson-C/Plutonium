//
// Created by jello_ on 7/3/2022.
//

#include "Buffers.h"


static VkPhysicalDeviceMemoryProperties s_MemoryProperties;

void InitializeBufferInfo(VkPhysicalDevice physicalDevice) { vkGetPhysicalDeviceMemoryProperties(physicalDevice, &s_MemoryProperties); }


static uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    for (uint32_t i = 0; i < s_MemoryProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (s_MemoryProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }
}

Buffer CreateBuffer(VkDevice device, VkDeviceSize size, U32 queueFamily, VkBufferUsageFlagBits usage, VkMemoryPropertyFlagBits memoryFlags) {
    Buffer Buffer;

    VkBufferCreateInfo bufferInfo;
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.pNext = VK_NULL_HANDLE;
    bufferInfo.flags = 0;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    bufferInfo.queueFamilyIndexCount = 1;
    bufferInfo.pQueueFamilyIndices = &queueFamily;

    vkCreateBuffer(device, &bufferInfo, VK_NULL_HANDLE, &Buffer.buffer);

    VkMemoryRequirements bufferRequirements;
    vkGetBufferMemoryRequirements(device, Buffer.buffer, &bufferRequirements);

    VkMemoryAllocateInfo bufferAlloc;
    bufferAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    bufferAlloc.pNext = VK_NULL_HANDLE;
    bufferAlloc.allocationSize = bufferRequirements.size;
    bufferAlloc.memoryTypeIndex = findMemoryType(bufferRequirements.memoryTypeBits, memoryFlags);

    vkAllocateMemory(device, &bufferAlloc, VK_NULL_HANDLE, &Buffer.memory);
    vkBindBufferMemory(device, Buffer.buffer, Buffer.memory, 0);


    return Buffer;
}

Buffer CreateBufferToGPU(VkDevice device, U32 queueFamily, VkDeviceSize size, VkBufferUsageFlagBits usageFlags, void* data) {
    Buffer StagingBuffer = CreateBuffer(device, size, queueFamily, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, CPU_COHERENT | CPU_VISIBLE);
    Buffer Buffer = CreateBuffer(device, size, queueFamily, usageFlags | VK_BUFFER_USAGE_TRANSFER_DST_BIT, GPU_LOCAL);

    void* bufferData;
    vkMapMemory(device, StagingBuffer.memory, 0, size, 0, &bufferData);
    memcpy(bufferData, data, (VkDeviceSize) size);
    vkUnmapMemory(device, StagingBuffer.memory);

    VkCommandBufferBeginInfo vertexCopyBeginfo;
    vertexCopyBeginfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    vertexCopyBeginfo.pNext = VK_NULL_HANDLE;
    vertexCopyBeginfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vertexCopyBeginfo.pInheritanceInfo = VK_NULL_HANDLE;

    VkCommandBuffer cmdBuffer = AcquireGlobalCommandBuffer(device);

    vkBeginCommandBuffer(cmdBuffer, &vertexCopyBeginfo);
    VkBufferCopy CopyInfo = {.size = size,.dstOffset = 0,.srcOffset = 0};
    vkCmdCopyBuffer(cmdBuffer, StagingBuffer.buffer, Buffer.buffer, 1, &CopyInfo);
    vkEndCommandBuffer(cmdBuffer);

    GlobalSubmit(cmdBuffer);

    destroyBuffer(device, &StagingBuffer);

    return Buffer;
}

UniformBuffer CreateUniformBuffers(VkDevice device, VkDeviceSize size, VkShaderStageFlagBits stage, U32 binding, U32 queueFamily) {
    UniformBuffer Buffer;
    Buffer.buffer = CreateBuffer(device, size, queueFamily, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,CPU_VISIBLE | CPU_COHERENT);
    Buffer.bufferInfo.buffer = Buffer.buffer.buffer;
    Buffer.bufferInfo.offset = 0;
    Buffer.bufferInfo.range = size;
    Buffer.descriptor = CreateSingleDescriptor(device, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, stage, binding);
    WriteDescriptor(device, Buffer.descriptor.set, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, binding, &Buffer.bufferInfo, VK_NULL_HANDLE);
    return Buffer;
}

void UploadDataToBuffer(VkDevice device, Buffer* buffer, VkDeviceSize size, void* data) {
    void* bufferData;
    vkMapMemory(device, buffer->memory, 0, size, 0, &bufferData);
    memcpy(bufferData, data, (VkDeviceSize)size);
    vkUnmapMemory(device, buffer->memory);
}

void UpdateUniform(VkDevice device, UniformBuffer* buffer, VkDeviceSize size, void* data) {
    UploadDataToBuffer(device, &buffer->buffer, size, data);
}

void BindUniform(VkCommandBuffer cmdBuffer, VkPipelineBindPoint bindPoint, VkPipelineLayout pipelineLayout, UniformBuffer buffer) {
    BindSingleDescriptor(cmdBuffer, bindPoint, pipelineLayout, buffer.descriptor);
}

void destroyBuffer(VkDevice device, Buffer* buffer) {
    vkDestroyBuffer(device, buffer->buffer, VK_NULL_HANDLE);
    vkFreeMemory(device, buffer->memory, VK_NULL_HANDLE);
}
