//
// Created by Owner on 10/16/2022.
//


#include "Descriptors.h"

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

PLCore_DescriptorSet PLCore_CreateDescriptorSets(VkDevice device, VkDescriptorType typeFlags, PLCore_DescriptorPoolAllocator allocator) {

    // Setting This To 20 Because I Dont Really Know What Else To Set It To
    // Binding Counts Should Really Never Be Over 3 (In My Mind)
    const uint32_t MAX_BINDINGS = 20;

    uint32_t bindingCount = 0;
    VkDescriptorSetLayoutBinding* bindings = malloc(sizeof(VkDescriptorSetLayoutBinding) * MAX_BINDINGS);


    // Search Through The Allocators Bindings To Find The Correct Descriptor Type
    for (int i = 0; i < allocator.bindingCount; i++) {
        if (allocator.types[i] & typeFlags) {
            bindings[i] = allocator.bindings[i];
            bindingCount++;
        }
    }


    VkDescriptorSetLayoutCreateInfo layoutInfo = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .pNext = VK_NULL_HANDLE,
            .flags = 0,
            .bindingCount = bindingCount,
            .pBindings = bindings,
    };
    VkDescriptorSetLayout layout;
    vkCreateDescriptorSetLayout(device, &layoutInfo, VK_NULL_HANDLE, &layout);

    VkDescriptorSetAllocateInfo allocInfo = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .pNext = VK_NULL_HANDLE,
            .descriptorSetCount = 1,
            .pSetLayouts = &layout,
            .descriptorPool = allocator.pool,
    };
    VkDescriptorSet set;
    vkAllocateDescriptorSets(device, &allocInfo, &set);

    PLCore_DescriptorSet descriptorSet = {
            .set = set,
            .layout = layout
    };
    return descriptorSet;
}

void PLCore_UpdateDescriptor(VkDevice device, VkDescriptorSet set, VkDescriptorType type, uint32_t dstBinding, VkDescriptorBufferInfo* bufferInfo, VkDescriptorImageInfo* imageInfo) {
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


PLCore_ReflectedDescriptorSet scanShaders(PLCore_RenderInstance instance, PLCore_ShaderModule module) {
    PLCore_ReflectedDescriptorSet reflectedSets;

    SpvReflectShaderModule mod;
    spvReflectCreateShaderModule(module.size, module.buffer, &mod);

    uint32_t descriptorCount = 0;
    spvReflectEnumerateDescriptorSets(&mod, &descriptorCount, VK_NULL_HANDLE);
    SpvReflectDescriptorSet** descriptorSet = malloc(sizeof(SpvReflectDescriptorSet*) * descriptorCount);
    spvReflectEnumerateDescriptorSets(&mod, &descriptorCount, descriptorSet);
    reflectedSets.descriptorSlots = descriptorCount;


    PLCore_DescriptorSet** returnSets = malloc(sizeof(PLCore_DescriptorSet*) * descriptorCount);
    reflectedSets.descriptorCount = malloc(sizeof(uint32_t) * descriptorCount);

    uint32_t allocCount = 0;
    for (int descriptorSlot = 0; descriptorSlot < descriptorCount; descriptorSlot++) {
        SpvReflectDescriptorSet* thisSet = &(*descriptorSet)[descriptorSlot];
        printf("Set: %i\n", thisSet->set);
        VkDescriptorSetLayoutBinding* bindings = malloc(sizeof(VkDescriptorSetLayoutBinding) * thisSet->binding_count);
        VkDescriptorPoolSize* sizes = malloc(sizeof(VkDescriptorPoolSize) * thisSet->binding_count);
        for (int descriptorBinding = 0; descriptorBinding < thisSet->binding_count; descriptorBinding++) {
            SpvReflectDescriptorBinding* thisBinding = (thisSet->bindings[descriptorBinding]);
            bindings[descriptorBinding] = (VkDescriptorSetLayoutBinding){
                    .binding = thisBinding->binding,
                    .descriptorType = (VkDescriptorType)thisBinding->descriptor_type,
                    .descriptorCount = (allocCount = thisBinding->count),
                    .stageFlags = module.stage,
                    .pImmutableSamplers = VK_NULL_HANDLE,
            };
            sizes[descriptorBinding] = (VkDescriptorPoolSize){
                .descriptorCount = thisBinding->count,
                .type = (VkDescriptorType)thisBinding->descriptor_type,
            };
            printf("\tType: %i\n", (VkDescriptorType)thisBinding->descriptor_type);
        }
        const uint32_t MAX_SETS = 100;
        VkDescriptorPoolCreateInfo poolInfo = {
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
                .pNext = VK_NULL_HANDLE,
                .flags = 0,
                .maxSets = MAX_SETS,
                .poolSizeCount = thisSet->binding_count,
                .pPoolSizes = sizes,
        };
        VkDescriptorSetLayoutCreateInfo layoutInfo = {
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
                .pNext = VK_NULL_HANDLE,
                .flags = 0,
                .bindingCount = thisSet->binding_count,
                .pBindings = bindings,
        };

        VkDescriptorPool pool;
        vkCreateDescriptorPool(instance.pl_device.device, &poolInfo, VK_NULL_HANDLE, &pool);
        VkDescriptorSetLayout layout;
        vkCreateDescriptorSetLayout(instance.pl_device.device, &layoutInfo, VK_NULL_HANDLE, &layout);



        VkDescriptorSetLayout* localLayouts = malloc(sizeof(VkDescriptorSetLayout) * allocCount);
        for (int i = 0; i < allocCount; i++) {
            localLayouts[i] = layout;
        }

        VkDescriptorSetAllocateInfo allocInfo = {
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
                .pNext = VK_NULL_HANDLE,
                .descriptorPool = pool,
                .descriptorSetCount = allocCount,
                .pSetLayouts = &layout,
        };
        VkDescriptorSet* set = malloc(sizeof(VkDescriptorSet) * allocCount);
        vkAllocateDescriptorSets(instance.pl_device.device, &allocInfo, set);

        PLCore_DescriptorSet* localSets = malloc(sizeof(allocCount));
        for (int i = 0; i < allocCount; i++) {
            localSets[i].set = set[i];
            localSets[i].layout = layout;
        }
        returnSets[descriptorSlot] = localSets;
        reflectedSets.descriptorCount[descriptorSlot] = allocCount;
    }
    reflectedSets.sets = returnSets;
    return reflectedSets;
}

