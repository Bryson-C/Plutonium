//
// Created by jello_ on 7/8/2022.
//

#ifndef PLUTONIUM_DESCRIPTORS_H
#define PLUTONIUM_DESCRIPTORS_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "vulkan/vulkan.h"

#ifndef U32
typedef uint32_t U32;
#endif

VkDescriptorSetLayoutBinding CreateNewBinding(U32 slot, VkDescriptorType type, U32 descriptorCount, VkShaderStageFlagBits stages);
VkDescriptorSetLayout CreateDescriptorLayout(VkDevice device, U32 bindingCount, VkDescriptorSetLayoutBinding* bindings);
VkDescriptorSet* CreateDescriptorSet(VkDevice device, U32 count, VkDescriptorType type, VkDescriptorSetLayout layouts, VkDescriptorPool* rPool);
void WriteDescriptor(VkDevice device, VkDescriptorSet set, VkDescriptorType type, U32 dstBinding, VkDescriptorBufferInfo* bufferInfo, VkDescriptorImageInfo* imageInfo);

void DestroyDescriptorPool(VkDevice device, VkDescriptorPool pool);
void DestroyDescriptorSets(VkDevice device, VkDescriptorSet* sets);
void DestroyDescriptorLayout(VkDevice device, VkDescriptorSetLayout layout);

typedef struct {
    VkDescriptorSetLayout layout;
    VkDescriptorPool pool;
    VkDescriptorSet set;
} SingleDescriptor;

SingleDescriptor CreateSingleDescriptor(VkDevice device, VkDescriptorType type, VkShaderStageFlagBits stage, U32 binding);
void BindSingleDescriptor(VkCommandBuffer cmdBuffer, VkPipelineBindPoint bindPoint, VkPipelineLayout pipelineLayout, SingleDescriptor descriptor);
void BindDescriptorSets(VkCommandBuffer cmdBuffer, VkPipelineBindPoint bindPoint, VkPipelineLayout pipelineLayout, U32 count, VkDescriptorSet* sets);
void DestroySingleDescriptor(VkDevice device, SingleDescriptor descriptor);

typedef struct {
    U32 arraySize;
    VkDescriptorSetLayout layout;
    VkDescriptorPool pool;
    VkDescriptorSet* set;
} DescriptorArray;

DescriptorArray CreateDescriptorArray(VkDevice device, VkDescriptorType type, VkShaderStageFlagBits stage, U32 binding, U32 count);



#endif //PLUTONIUM_DESCRIPTORS_H
