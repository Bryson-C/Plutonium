//
// Created by Owner on 10/16/2022.
//

#ifndef PLUTONIUM_DESCRIPTORS_H
#define PLUTONIUM_DESCRIPTORS_H

#include <stdlib.h>


#include "vulkan/vulkan.h"
#include "SpirvReflection.h"

#include "PlutoniumCore.h"

typedef struct {
    // Creation Information
    VkDescriptorPoolSize* sizes;
    uint32_t poolSizeCount;
    VkDescriptorSetLayoutBinding* bindings;
    uint32_t bindingCount;
    VkDescriptorType* types;
    uint32_t typeCount;
    uint32_t maxDescriptorSets;
    // Data Structures
    VkDescriptorPool pool;
} PLCore_DescriptorPoolAllocator;

typedef struct {
    VkDescriptorSetLayout layout;
    VkDescriptorSet set;
} PLCore_DescriptorSet;

typedef struct {
    // Actual Vulkan Objects
    PLCore_DescriptorSet** sets;
    // How Many Descriptors Per Set
    uint32_t* descriptorCount;
    // How Many Sets
    uint32_t descriptorSlots;
} PLCore_ReflectedDescriptorSet;

PLCore_DescriptorPoolAllocator PLCore_CreateDescriptorPoolAllocator(uint32_t descriptorSlot, VkDescriptorType* types, uint32_t* descriptorSetCount, uint32_t count, uint32_t maxDescriptorSets, VkShaderStageFlagBits shaderStage);
PLCore_DescriptorPoolAllocator PLCore_CreateDescriptorPoolFromAllocator(VkDevice device, PLCore_DescriptorPoolAllocator allocator);
PLCore_DescriptorSet PLCore_CreateDescriptorSets(VkDevice device, VkDescriptorType typeFlags, PLCore_DescriptorPoolAllocator allocator);
void PLCore_UpdateDescriptor(VkDevice device, VkDescriptorSet set, VkDescriptorType type, uint32_t dstBinding, VkDescriptorBufferInfo* bufferInfo, VkDescriptorImageInfo* imageInfo);

PLCore_ReflectedDescriptorSet scanShaders(PLCore_RenderInstance instance, PLCore_ShaderModule module);

#endif //PLUTONIUM_DESCRIPTORS_H
