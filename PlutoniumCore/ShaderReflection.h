//
// Created by Owner on 10/5/2022.
//

#ifndef PLUTONIUM_SHADERREFLECTION_H
#define PLUTONIUM_SHADERREFLECTION_H

#include "PlutoniumCore.h"
#include "SpirvReflection.h"

typedef struct {
    uint32_t setCount;
    VkDescriptorSet* descriptorSets;
    VkDescriptorSetLayout* layouts;
    VkDescriptorPool pool;
} DescriptorReturnData;

inline DescriptorReturnData scanShader(PLCore_RenderInstance instance, PLCore_ShaderModule shader) {

    SpvReflectShaderModule module;
    spvReflectCreateShaderModule(shader.size, shader.buffer, &module);

    uint32_t descriptorSetCount = 0;
    spvReflectEnumerateDescriptorSets(&module, &descriptorSetCount, VK_NULL_HANDLE);

    SpvReflectDescriptorSet** sets           = malloc(sizeof(SpvReflectShaderModule)         * descriptorSetCount);
    VkDescriptorSet* descriptorSets         = malloc(sizeof(VkDescriptorSetLayoutBinding)   * descriptorSetCount);
    VkDescriptorSetLayout* layouts          = malloc(sizeof(VkDescriptorSetLayoutBinding)   * descriptorSetCount);

    spvReflectEnumerateDescriptorSets(&module, &descriptorSetCount, sets);

    uint32_t sizeCount = 0;
    VkDescriptorPoolSize sizes[100];
    VkDescriptorPool pool;

    // loop to get the descriptor pool info
    for (int i = 0; i < descriptorSetCount; i++) {
        for (int j = 0; j < sets[i]->binding_count; j++) {
            sizes[i].descriptorCount = 1;
            sizes[i].type = (VkDescriptorType)sets[i]->bindings[j]->descriptor_type;
            sizeCount++;
        }
    }
    VkDescriptorPoolCreateInfo poolInfo = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .pNext = VK_NULL_HANDLE,
            .flags = 0,
            .maxSets = descriptorSetCount,
            .poolSizeCount = sizeCount,
            .pPoolSizes = sizes,
    };
    vkCreateDescriptorPool(instance.pl_device.device, &poolInfo, VK_NULL_HANDLE, &pool);

    for (int i = 0; i < descriptorSetCount; i++) {
        VkDescriptorSetLayoutBinding* bindings = malloc(sizeof(VkDescriptorSetLayoutBinding) * sets[i]->binding_count);
        uint32_t setsInBinding = 0;
        for (int j = 0; j < sets[i]->binding_count; j++) {
            bindings[j].binding = sets[i]->bindings[j]->binding;
            bindings[j].descriptorCount = sets[i]->bindings[j]->binding;
            bindings[j].descriptorType = (VkDescriptorType)sets[i]->bindings[j]->descriptor_type;
            bindings[j].stageFlags = shader.stage;
            bindings[j].pImmutableSamplers = VK_NULL_HANDLE;
            setsInBinding = sets[i]->bindings[j]->count;
            printf("Descriptor '%s' at Set: %i, Array of: %i\n", sets[i]->bindings[j]->name, sets[i]->bindings[j]->set, sets[i]->bindings[j]->count);
        }
        VkDescriptorSetLayoutCreateInfo layoutInfo = {
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
                .pNext = VK_NULL_HANDLE,
                .flags = 0,
                .bindingCount = sets[i]->binding_count,
                .pBindings = bindings,
        };
        vkCreateDescriptorSetLayout(instance.pl_device.device, &layoutInfo, VK_NULL_HANDLE, &(layouts[i]));

        VkDescriptorSetLayout* localLayouts = malloc(sizeof(VkDescriptorSetLayoutBinding) * setsInBinding);
        for (int k = 0; k < setsInBinding; k++)
            localLayouts[k] = *layouts;

        VkDescriptorSetAllocateInfo allocInfo = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .pNext = VK_NULL_HANDLE,
            .descriptorSetCount = setsInBinding,
            .pSetLayouts = localLayouts,
            .descriptorPool = pool,
        };
        vkAllocateDescriptorSets(instance.pl_device.device, &allocInfo, &descriptorSets[i]);
    }

    return (DescriptorReturnData) {
        .setCount = descriptorSetCount,
        .descriptorSets = descriptorSets,
        .layouts = layouts,
        .pool = pool,
    };
    //spvReflectEnumeratePushConstantBlocks();
    //spvReflectEnumerateInputVariables();

}



#endif //PLUTONIUM_SHADERREFLECTION_H
