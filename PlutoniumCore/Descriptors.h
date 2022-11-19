//
// Created by Owner on 10/16/2022.
//

#ifndef PLUTONIUM_DESCRIPTORS_H
#define PLUTONIUM_DESCRIPTORS_H

#include <stdlib.h>


#include "vulkan/vulkan.h"
#include "SpirvReflection.h"

#include "PlutoniumCore.h"


//#define DEBUG

#ifndef PLCORE_RESULT
#ifdef DEBUG
    #define PLCORE_RESULT(name, function) printf("%s: %s\n", name, ( function == VK_SUCCESS) ? "Success" : "Failure");
#else
    #define PLCORE_RESULT(name, function) function;
#endif
#endif


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
    VkDescriptorSetLayoutBinding* bindings;
    uint32_t bindingCount;

    VkDescriptorSetLayout layout;
    VkDescriptorSet set;
    VkWriteDescriptorSet write;

    uint32_t slot;
} PLCore_DescriptorSet;

typedef struct {
    void* nulldata;
} PLCore_Priv_DescriptorReflectionData;


PLCore_DescriptorPoolAllocator PLCore_CreateDescriptorPoolAllocator(uint32_t descriptorSlot, VkDescriptorType* types, uint32_t* descriptorSetCount, uint32_t count, uint32_t maxDescriptorSets, VkShaderStageFlagBits shaderStage);
PLCore_DescriptorPoolAllocator PLCore_CreateDescriptorPoolFromAllocator(VkDevice device, PLCore_DescriptorPoolAllocator allocator);
PLCore_DescriptorSet PLCore_CreateDescriptorSets(VkDevice device, VkDescriptorType typeFlags, PLCore_DescriptorPoolAllocator allocator);
void PLCore_UpdateDescriptor(PLCore_RenderInstance instance, VkDescriptorSet set, VkDescriptorType type, uint32_t dstBinding, VkDescriptorBufferInfo* bufferInfo, VkDescriptorImageInfo* imageInfo);

/*
PLCore_ReflectedDescriptorSet scanShaders(PLCore_RenderInstance instance, PLCore_ShaderModule module);

PLCore_EXP_shaderDescriptors PLCore_EXP_ReflectShader(PLCore_RenderInstance instance, PLCore_ShaderModule module);
*/

VkDescriptorSetLayoutBinding PLCore_CreateDescriptorSetLayoutBinding(uint32_t slot, uint32_t count, VkDescriptorType type, VkShaderStageFlags stage);
VkDescriptorSet PLCore_CreateDescriptorSetAdvanced(PLCore_RenderInstance instance, VkDescriptorPool pool, uint32_t bindingCount, VkDescriptorSetLayoutBinding* bindings, VkShaderStageFlags stage, VkDescriptorSetLayout* layout);

VkDescriptorPool PLCore_CreateGeneralizedDescriptorPool(PLCore_RenderInstance instance);

VkDescriptorSet PLCore_CreateDescriptorSet(PLCore_RenderInstance instance, VkDescriptorPool pool, uint32_t slot, VkDescriptorType type, VkShaderStageFlags stage, VkDescriptorSetLayout* layout);

#endif //PLUTONIUM_DESCRIPTORS_H
