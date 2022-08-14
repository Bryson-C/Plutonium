//
// Created by jello_ on 7/3/2022.
//

#ifndef PLUTONIUM_GLOBALS_H
#define PLUTONIUM_GLOBALS_H

// TODO: Move Into RenderState

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "vulkan/vulkan.h"
#include "Descriptors.h"

#ifndef U32
#define U32 uint32_t
#endif

// global transfer command pool
typedef struct { VkCommandBuffer buffer; VkFence fence; } CmdBufferWithFence;

typedef struct {
    VkDevice ref_Device;
    VkCommandPool pool;
    VkQueue submissionQueue;
    VkFence submitFence;
} GlobalSubmissionQueue;

void CreateGlobalSubmissionQueue(VkDevice device, VkQueue queue, U32 queueFamily);
void DestroyGlobalSubmissionQueue();
void GlobalSubmit(VkCommandBuffer cmdBuffer);
VkCommandBuffer AcquireGlobalCommandBuffer();

//TODO: global descriptor pool
typedef struct {
    VkDescriptorSet set;
} GlobalDescriptorPool;

void CreateGlobalDescriptorPool();

//TODO: global pipeline manager

void CreateGlobalPipelineManager();






#endif //PLUTONIUM_GLOBALS_H
