//
// Created by jello_ on 7/3/2022.
//

#include "Globals.h"

static GlobalSubmissionQueue s_GlobalSubmissionQueue;
void CreateGlobalSubmissionQueue(VkDevice device, VkQueue queue, U32 queueFamily) {
    VkFenceCreateInfo fenceInfo;
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.pNext = VK_NULL_HANDLE;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    vkCreateFence(device, &fenceInfo, VK_NULL_HANDLE, &s_GlobalSubmissionQueue.submitFence);

    vkWaitForFences(device, 1, &s_GlobalSubmissionQueue.submitFence, VK_TRUE, UINT64_MAX);
    vkResetFences(device, 1, &s_GlobalSubmissionQueue.submitFence);

    VkCommandPoolCreateInfo poolInfo;
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.pNext = VK_NULL_HANDLE;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamily;
    vkCreateCommandPool(device, &poolInfo, VK_NULL_HANDLE, &s_GlobalSubmissionQueue.pool);

    s_GlobalSubmissionQueue.submissionQueue = queue;
    s_GlobalSubmissionQueue.ref_Device = device;
}
void DestroyGlobalSubmissionQueue() {
    vkDestroyCommandPool(s_GlobalSubmissionQueue.ref_Device, s_GlobalSubmissionQueue.pool, VK_NULL_HANDLE);
}
VkCommandBuffer AcquireGlobalCommandBuffer() {
    VkCommandBuffer cmdBuffer;
    VkCommandBufferAllocateInfo allocateInfo;
    allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocateInfo.pNext = VK_NULL_HANDLE;
    allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocateInfo.commandBufferCount = 1;
    allocateInfo.commandPool = s_GlobalSubmissionQueue.pool;
    VkResult result = vkAllocateCommandBuffers(s_GlobalSubmissionQueue.ref_Device, &allocateInfo, &cmdBuffer);
    if (result != VK_SUCCESS) {
        fprintf(stderr, "Failure Creating Command Buffer!\n");
    }
    return cmdBuffer;
}
void GlobalSubmit(VkCommandBuffer cmdBuffer) {
    VkSubmitInfo submitInfo;
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pNext = VK_NULL_HANDLE;
    submitInfo.waitSemaphoreCount = 0;
    submitInfo.pWaitSemaphores = VK_NULL_HANDLE;
    submitInfo.pWaitDstStageMask = VK_NULL_HANDLE;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmdBuffer;
    submitInfo.signalSemaphoreCount = 0;
    submitInfo.pSignalSemaphores = VK_NULL_HANDLE;

    vkQueueSubmit(s_GlobalSubmissionQueue.submissionQueue, 1, &submitInfo, s_GlobalSubmissionQueue.submitFence);
    vkWaitForFences(s_GlobalSubmissionQueue.ref_Device, 1, &s_GlobalSubmissionQueue.submitFence, VK_TRUE, UINT64_MAX);
    vkResetFences(s_GlobalSubmissionQueue.ref_Device, 1, &s_GlobalSubmissionQueue.submitFence);
}


static GlobalDescriptorPool s_GlobalDescriptorPool;
void CreateGlobalDescriptorPool() {}

