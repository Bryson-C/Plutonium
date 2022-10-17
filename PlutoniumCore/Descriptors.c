//
// Created by Owner on 10/16/2022.
//

#include "vulkan/vulkan.h"
#include <stdlib.h>

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

// `types` Needs To Be An Array
// `descriptorSetCount` Needs To Be An Array
// `count` Specifies How Large The `descriptorSetCount` and `types` Array Are: `descriptorSetCount` and `types` Should Be The Same Size
PLCore_DescriptorPoolAllocator PLCore_CreateDescriptorPoolAllocator(uint32_t descriptorSlot, VkDescriptorType* types, uint32_t* descriptorSetCount, uint32_t count, uint32_t maxDescriptorSets, VkShaderStageFlagBits shaderStage) {

    static const uint32_t DESCRIPTOR_ALLOC_COUNT_PADDING = 5;
    VkDescriptorPoolSize* sizes = malloc(sizeof(VkDescriptorPoolSize) * count);
    VkDescriptorSetLayoutBinding* bindings = malloc(sizeof(VkDescriptorSetLayoutBinding) * count);

    for (int i = 0; i < count; i++) {
        sizes[i] = (VkDescriptorPoolSize){.descriptorCount = (descriptorSetCount[i] + DESCRIPTOR_ALLOC_COUNT_PADDING), .type = types[i] };
        bindings[i] = (VkDescriptorSetLayoutBinding){
            .binding = descriptorSlot,
            .descriptorType = types[i],
            .descriptorCount = descriptorSetCount[i],
            .stageFlags = shaderStage,
            .pImmutableSamplers = VK_NULL_HANDLE,
        };
    }

    return (PLCore_DescriptorPoolAllocator) {
            .sizes = sizes,
            .poolSizeCount = count,
            .bindings = bindings,
            .bindingCount = count,
            .types = types,
            .typeCount = count,
            .maxDescriptorSets = maxDescriptorSets,
    };
}

PLCore_DescriptorPoolAllocator PLCore_CreateDescriptorPoolFromAllocator(VkDevice device, PLCore_DescriptorPoolAllocator allocator) {
    VkDescriptorPoolCreateInfo info = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .pNext = VK_NULL_HANDLE,
            .flags = 0,
            .maxSets = allocator.maxDescriptorSets,
            .poolSizeCount = allocator.poolSizeCount,
            .pPoolSizes = allocator.sizes,
    };
    vkCreateDescriptorPool(device, &info, VK_NULL_HANDLE, &allocator.pool);
    // This Returns A Local Copy
    // It Does This So You Can Chain Functions Together
    // Problems:
    //      If You Expect The Function To Return Nothing The Allocator Will Be Invalid For Creaing Descriptor Sets
    return allocator;
}

PLCore_DescriptorSet PLCore_CreateDescriptorSets(VkDevice device, VkDescriptorType type, PLCore_DescriptorPoolAllocator allocator) {

    // Search Through The Allocators Bindings To Find The Correct Descriptor Type
    for (int i = 0; i < allocator.bindingCount; i++) {
        //if (allocator.types[i] & type)
    }


    VkDescriptorSetLayoutCreateInfo layoutInfo = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .pNext = VK_NULL_HANDLE,
            .flags = 0,
            .bindingCount = ,
            .pBindings = ,
    }
}


