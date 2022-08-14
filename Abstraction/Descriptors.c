//
// Created by jello_ on 7/8/2022.
//

#include "Descriptors.h"
#include "Abstractions.h"


VkDescriptorSetLayoutBinding CreateNewBinding(U32 slot, VkDescriptorType type, U32 descriptorCount, VkShaderStageFlagBits stages) {
    VkDescriptorSetLayoutBinding binding;
    binding.binding = slot;
    binding.descriptorType = type;
    binding.descriptorCount = descriptorCount;
    binding.stageFlags = stages;
    binding.pImmutableSamplers = VK_NULL_HANDLE;
    return binding;
}

VkDescriptorSetLayout CreateDescriptorLayout(VkDevice device, U32 bindingCount, VkDescriptorSetLayoutBinding* bindings) {
    VkDescriptorSetLayout layout;
    VkDescriptorSetLayoutCreateInfo layoutInfo;
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.pNext = VK_NULL_HANDLE;
    layoutInfo.flags = 0;
    layoutInfo.bindingCount = bindingCount;
    layoutInfo.pBindings = bindings;

    vkCreateDescriptorSetLayout(device, &layoutInfo, VK_NULL_HANDLE, &layout);
    return layout;
}

VkDescriptorPool CreateDescriptorPool(VkDevice device, U32 sets, VkDescriptorType type) {
    VkDescriptorPool pool;

    VkDescriptorPoolSize size;
    size.type = type;
    size.descriptorCount = sets;

    VkDescriptorPoolCreateInfo poolInfo;
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.pNext = VK_NULL_HANDLE;
    poolInfo.flags = 0;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &size;
    poolInfo.maxSets = sets;

    vkCreateDescriptorPool(device, &poolInfo, VK_NULL_HANDLE, &pool);
    return pool;
}

VkDescriptorSet* CreateDescriptorSet(VkDevice device, U32 count, VkDescriptorType type, VkDescriptorSetLayout layout, VkDescriptorPool* rPool) {
    VkDescriptorSet* sets = malloc(sizeof(VkDescriptorSet) * count);

    *rPool = CreateDescriptorPool(device, count, type);

    VkDescriptorSetLayout* descriptorLayouts = malloc(sizeof(VkDescriptorSetLayout) * count);
    for (U32 i = 0; i < count; i++) descriptorLayouts[i] = layout;

    VkDescriptorSetAllocateInfo allocInfo;
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.pNext = VK_NULL_HANDLE;
    allocInfo.pSetLayouts = descriptorLayouts;
    allocInfo.descriptorSetCount = count;
    allocInfo.descriptorPool = *rPool;

    vkAllocateDescriptorSets(device, &allocInfo, sets);

    free(descriptorLayouts);

    return sets;
}

void WriteDescriptor(VkDevice device, VkDescriptorSet set, VkDescriptorType type, U32 dstBinding, VkDescriptorBufferInfo* bufferInfo, VkDescriptorImageInfo* imageInfo) {
    VkWriteDescriptorSet write;
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.pNext = VK_NULL_HANDLE;
    write.dstSet = set;
    write.dstBinding = dstBinding;
    write.dstArrayElement = 0;
    write.descriptorCount = 1;
    write.descriptorType = type;
    write.pImageInfo = imageInfo;
    write.pBufferInfo = bufferInfo;
    write.pTexelBufferView = VK_NULL_HANDLE;

    vkUpdateDescriptorSets(device, 1, &write, 0, VK_NULL_HANDLE);
}

SingleDescriptor CreateSingleDescriptor(VkDevice device, VkDescriptorType type, VkShaderStageFlagBits stage, U32 binding) {
    SingleDescriptor descriptor;

    VkDescriptorSetLayoutBinding setBinding;
    setBinding.binding = binding;
    setBinding.descriptorType = type;
    setBinding.descriptorCount = 1;
    setBinding.stageFlags = stage;
    setBinding.pImmutableSamplers = VK_NULL_HANDLE;


    VkDescriptorSetLayoutCreateInfo layoutInfo;
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.pNext = VK_NULL_HANDLE;
    layoutInfo.flags = 0;
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &setBinding;

    vkCreateDescriptorSetLayout(device, &layoutInfo, VK_NULL_HANDLE, &descriptor.layout);

    VkDescriptorPoolSize poolSize;
    poolSize.descriptorCount = 1;
    poolSize.type = type;

    VkDescriptorPoolCreateInfo poolInfo;
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.pNext = VK_NULL_HANDLE;
    poolInfo.flags = 0;
    poolInfo.maxSets = 1;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;

    vkCreateDescriptorPool(device, &poolInfo, VK_NULL_HANDLE, &descriptor.pool);

    VkDescriptorSetAllocateInfo allocInfo;
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.pNext = VK_NULL_HANDLE;
    allocInfo.descriptorPool = descriptor.pool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &descriptor.layout;

    vkAllocateDescriptorSets(device, &allocInfo, &descriptor.set);
    return descriptor;
}

void BindSingleDescriptor(VkCommandBuffer cmdBuffer, VkPipelineBindPoint bindPoint, VkPipelineLayout pipelineLayout,  SingleDescriptor descriptor) {
    vkCmdBindDescriptorSets(cmdBuffer, bindPoint, pipelineLayout, 0, 1, &descriptor.set, 0, VK_NULL_HANDLE);
}

void BindDescriptorSets(VkCommandBuffer cmdBuffer, VkPipelineBindPoint bindPoint, VkPipelineLayout pipelineLayout, U32 count, VkDescriptorSet* sets) {
    vkCmdBindDescriptorSets(cmdBuffer, bindPoint, pipelineLayout, 0, count, sets, 0, VK_NULL_HANDLE);
}


DescriptorArray CreateDescriptorArray(VkDevice device, VkDescriptorType type, VkShaderStageFlagBits stage, U32 binding, U32 count) {
    DescriptorArray descriptorArray;
    descriptorArray.arraySize = count;

    VkDescriptorSetLayoutBinding setBinding;
    setBinding.binding = binding;
    setBinding.descriptorType = type;
    setBinding.descriptorCount = count;
    setBinding.stageFlags = stage;
    setBinding.pImmutableSamplers = VK_NULL_HANDLE;


    VkDescriptorSetLayoutCreateInfo layoutInfo;
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.pNext = VK_NULL_HANDLE;
    layoutInfo.flags = 0;
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &setBinding;

    vkCreateDescriptorSetLayout(device, &layoutInfo, VK_NULL_HANDLE, &descriptorArray.layout);

    VkDescriptorPoolSize poolSize;
    poolSize.descriptorCount = count;
    poolSize.type = type;

    VkDescriptorPoolCreateInfo poolInfo;
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.pNext = VK_NULL_HANDLE;
    poolInfo.flags = 0;
    poolInfo.maxSets = count;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;

    vkCreateDescriptorPool(device, &poolInfo, VK_NULL_HANDLE, &descriptorArray.pool);

    VkDescriptorSetLayout* layouts = malloc(sizeof(VkDescriptorSetLayout) * count);
    for (U32 i = 0; i < count; i++) layouts[i] = descriptorArray.layout;

    VkDescriptorSetAllocateInfo allocInfo;
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.pNext = VK_NULL_HANDLE;
    allocInfo.descriptorPool = descriptorArray.pool;
    allocInfo.descriptorSetCount = count;
    allocInfo.pSetLayouts = layouts;

    VkResult res = vkAllocateDescriptorSets(device, &allocInfo, descriptorArray.set);
    return descriptorArray;
}



// TODO: Create Destruction Functions
void DestroyDescriptorPool(VkDevice device, VkDescriptorPool pool) {}
void DestroyDescriptorSets(VkDevice device, VkDescriptorSet* sets) {}
void DestroyDescriptorLayout(VkDevice device, VkDescriptorSetLayout layout) {}
void DestroySingleDescriptor(VkDevice device, SingleDescriptor descriptor) {}


