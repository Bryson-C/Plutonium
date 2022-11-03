//
// Created by Owner on 10/16/2022.
//


#include "Descriptors.h"

// `types` Needs To Be An Array
// `descriptorSetCount` Needs To Be An Array
// `count` Specifies How Large The `descriptorSetCount` and `types` Array Are: `descriptorSetCount` and `types` Should Be The Same Size
PLCore_DescriptorPoolAllocator PLCore_CreateDescriptorPoolAllocator(uint32_t descriptorSlot, VkDescriptorType* types, uint32_t* descriptorSetCount, uint32_t count, uint32_t maxDescriptorSets, VkShaderStageFlagBits shaderStage) {

    static const uint32_t DESCRIPTOR_ALLOC_COUNT_PADDING = 5;
    VkDescriptorPoolSize* sizes = (VkDescriptorPoolSize*)malloc(sizeof(VkDescriptorPoolSize) * count);
    VkDescriptorSetLayoutBinding* bindings = (VkDescriptorSetLayoutBinding*)malloc(sizeof(VkDescriptorSetLayoutBinding) * count);

    for (int i = 0; i < count; i++) {
        sizes[i].descriptorCount = (descriptorSetCount[i] + DESCRIPTOR_ALLOC_COUNT_PADDING);
        sizes[i].type = types[i];

        bindings[i].binding = descriptorSlot;
        bindings[i].descriptorType = types[i];
        bindings[i].descriptorCount = descriptorSetCount[i];
        bindings[i].stageFlags = (VkShaderStageFlags)shaderStage;
        bindings[i].pImmutableSamplers = VK_NULL_HANDLE;
    }

    PLCore_DescriptorPoolAllocator poolAllocator;
    poolAllocator.sizes = sizes;
    poolAllocator.poolSizeCount = count;
    poolAllocator.bindings = bindings;
    poolAllocator.bindingCount = count;
    poolAllocator.types = types;
    poolAllocator.typeCount = count;
    poolAllocator.maxDescriptorSets = maxDescriptorSets;

    return poolAllocator;
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
    VkDescriptorSetLayoutBinding* bindings = (VkDescriptorSetLayoutBinding*)malloc(sizeof(VkDescriptorSetLayoutBinding) * MAX_BINDINGS);


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

    VkDescriptorSetAllocateInfo allocInfo;
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.pNext = VK_NULL_HANDLE;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &layout;
    allocInfo.descriptorPool = allocator.pool;

    VkDescriptorSet set;
    vkAllocateDescriptorSets(device, &allocInfo, &set);

    PLCore_DescriptorSet descriptorSet;
    descriptorSet.set = set;
    descriptorSet.layout = layout;

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
    SpvReflectDescriptorSet** descriptorSets = malloc(sizeof(SpvReflectDescriptorSet*) * descriptorCount);
    spvReflectEnumerateDescriptorSets(&mod, &descriptorCount, descriptorSets);
    reflectedSets.descriptorSlots = descriptorCount;
    reflectedSets.descriptorCount = malloc(sizeof(uint32_t) * descriptorCount);

    const uint32_t DESCRIPTORS_PER_TYPE = 25;
    VkDescriptorPoolSize sizes[11] = {
            {VK_DESCRIPTOR_TYPE_SAMPLER, DESCRIPTORS_PER_TYPE},
            {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, DESCRIPTORS_PER_TYPE},
            {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, DESCRIPTORS_PER_TYPE},
            {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, DESCRIPTORS_PER_TYPE},
            {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, DESCRIPTORS_PER_TYPE},
            {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, DESCRIPTORS_PER_TYPE},
            {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, DESCRIPTORS_PER_TYPE},
            {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, DESCRIPTORS_PER_TYPE},
            {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, DESCRIPTORS_PER_TYPE},
            {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, DESCRIPTORS_PER_TYPE},
            {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, DESCRIPTORS_PER_TYPE},
    };
    VkDescriptorPoolCreateInfo poolInfo;
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.pNext = VK_NULL_HANDLE;
    poolInfo.flags = 0;
    poolInfo.poolSizeCount = 11;
    poolInfo.pPoolSizes = sizes;
    // Descriptor Types * DESCRIPTORS_PER_TYPE
    poolInfo.maxSets = 11 * DESCRIPTORS_PER_TYPE;

    VkDescriptorPool pool;
    vkCreateDescriptorPool(instance.pl_device.device, &poolInfo, VK_NULL_HANDLE, &pool);
    reflectedSets.pool = pool;

    for (int set = 0; set < descriptorCount; set++) {
        printf("Set: %u\n", descriptorSets[set]->set);
        uint32_t allocCount = 0;
        VkDescriptorSetLayoutBinding* bindings = malloc(sizeof(VkDescriptorSetLayoutBinding) * descriptorSets[set]->binding_count);
        for (int binding = 0; binding < descriptorSets[set]->binding_count; binding++) {
            printf("\tBinding: \"%s\"\n", descriptorSets[set]->bindings[binding]->name);

            bindings[binding].binding = descriptorSets[set]->bindings[binding]->binding;
            printf("\tBinding# : %u\n", descriptorSets[set]->bindings[binding]->binding);
            bindings[binding].descriptorCount = descriptorSets[set]->bindings[binding]->count;
            bindings[binding].descriptorType = (VkDescriptorType)descriptorSets[set]->bindings[binding]->descriptor_type;
            bindings[binding].pImmutableSamplers = VK_NULL_HANDLE;
            bindings[binding].stageFlags = module.stage;

            allocCount += descriptorSets[set]->bindings[binding]->count;
        }
        printf("\tBinding Count: %u\n", descriptorSets[set]->binding_count);
        printf("\tAllocating: %u\n", allocCount);

        VkDescriptorSetLayout layout;
        VkDescriptorSetLayoutCreateInfo layoutInfo;
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.pNext = VK_NULL_HANDLE;
        layoutInfo.flags = 0;
        layoutInfo.bindingCount = descriptorSets[set]->binding_count;
        layoutInfo.pBindings = bindings;
        vkCreateDescriptorSetLayout(instance.pl_device.device, &layoutInfo, VK_NULL_HANDLE, &layout);

        VkDescriptorSetLayout* layoutCopies = malloc(sizeof(VkDescriptorSetLayout) * allocCount);
        for (int i = 0; i < allocCount; i++) {
            layoutCopies[i] = layout;
        }

        VkDescriptorSetAllocateInfo allocInfo;
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.pNext = VK_NULL_HANDLE;
        allocInfo.descriptorPool = reflectedSets.pool;
        allocInfo.pSetLayouts = layoutCopies;
        allocInfo.descriptorSetCount = allocCount;

        VkDescriptorSet* sets = malloc(sizeof(VkDescriptorSet) * allocCount);
        vkAllocateDescriptorSets(instance.pl_device.device, &allocInfo, sets);

        reflectedSets.sets = (PLCore_DescriptorSet**)malloc(sizeof(PLCore_DescriptorSet*) * descriptorCount);

        reflectedSets.descriptorCount[set] = allocCount;
        for (int i = 0; i < allocCount; i++) {
            reflectedSets.sets[set] = (PLCore_DescriptorSet*)malloc(sizeof(PLCore_DescriptorSet*));
            (*reflectedSets.sets[set]).set = sets[i];
            (*reflectedSets.sets[set]).layout = layoutCopies[i];
            (*reflectedSets.sets[set]).slot = descriptorSets[set]->set;
        }
    }
    return reflectedSets;
}

