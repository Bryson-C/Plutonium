//
// Created by Owner on 10/5/2022.
//

#ifndef PLUTONIUM_SHADERREFLECTION_H
#define PLUTONIUM_SHADERREFLECTION_H

#include "PlutoniumCore.h"
#include "SpirvReflection.h"

typedef struct {
    const char* name;
    uint32_t set;
    uint32_t count;
    VkDescriptorType type;
    VkShaderStageFlagBits stage;
} DescriptorInfo;

typedef struct {
    PLCore_Descriptor* descriptors;
    PLCore_DescriptorPool* pools;
} DescriptorReturnData;


inline DescriptorReturnData scanShader(PLCore_RenderInstance instance, PLCore_ShaderModule shader) {

    // Create The Module From The Shader To Be Used For Reflection
    SpvReflectShaderModule module;
    spvReflectCreateShaderModule(shader.size, shader.buffer, &module);

    // Get Descriptor Information From The Shader
    // Getting Set Count
    uint32_t descriptorSetCount = 0;
    spvReflectEnumerateDescriptorSets(&module, &descriptorSetCount, VK_NULL_HANDLE);
    // Allocating Set Information
    SpvReflectDescriptorSet** reflectSets = malloc(sizeof(SpvReflectShaderModule) * descriptorSetCount);
    spvReflectEnumerateDescriptorSets(&module, &descriptorSetCount, reflectSets);

    // This Number Is Multiplied By The Allocation Max Count For More Space Just In Case
    // For Example:
    // malloc(sizeof(struct) * (count + padding));
    static const uint32_t DESCRIPTOR_ALLOC_COUNT_PADDING = 5;

    PLCore_Descriptor* descriptors = malloc(sizeof(PLCore_Descriptor) * descriptorSetCount);
    PLCore_DescriptorPool* descriptorPools = malloc(sizeof(PLCore_Descriptor) * descriptorSetCount);

    for (int descriptorSlot = 0; descriptorSlot < descriptorSetCount; descriptorSlot++) {
        printf("Descriptor %i:\n", reflectSets[descriptorSlot]->set);
        VkDescriptorPoolSize* sizes = malloc(sizeof(VkDescriptorPoolSize) * reflectSets[descriptorSlot]->binding_count);
        uint32_t totalSetCount = 0;
        for (int slotBinding = 0; slotBinding < reflectSets[descriptorSlot]->binding_count; slotBinding++) {
            printf("\tSet: %i\n", reflectSets[descriptorSlot]->bindings[slotBinding]->set);
            printf("\tType: %i\n", (VkDescriptorType)reflectSets[descriptorSlot]->bindings[slotBinding]->descriptor_type);
            printf("\tName: \"%s\"\n", reflectSets[descriptorSlot]->bindings[slotBinding]->name);
            totalSetCount += reflectSets[descriptorSlot]->bindings[slotBinding]->count;
            sizes[slotBinding].type = (VkDescriptorType)reflectSets[descriptorSlot]->bindings[slotBinding]->descriptor_type;
            sizes[slotBinding].descriptorCount = reflectSets[descriptorSlot]->bindings[slotBinding]->count;
        }
        // TODO: FIX
        // descriptorPools[descriptorSlot] = PLCore_CreateDescriptorPoolDetailed(instance, totalSetCount + DESCRIPTOR_ALLOC_COUNT_PADDING, reflectSets[descriptorSlot]->binding_count, sizes);
        // descriptors[descriptorSlot].sets = PLCore_CreateDescriptorFromPool(instance, descriptorPools[descriptorSlot], totalSetCount, );
        // descriptors[descriptorSlot].count++;
    }







    spvReflectDestroyShaderModule(&module);

    DescriptorReturnData returnData;
    return returnData;
    //spvReflectEnumeratePushConstantBlocks();
    //spvReflectEnumerateInputVariables();

}



#endif //PLUTONIUM_SHADERREFLECTION_H
