#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#include "Abstraction/Abstractions.h"

#include "vulkan/vulkan.h"
#include "shaderc.h"
#include "status.h"
#include "glfw3.h"

typedef uint32_t U32;


static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData) {

    if (messageSeverity > VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
        printf("[Vk]: %s\n\n",pCallbackData->pMessage);

    return VK_FALSE;
}

const char* shaderCompilationErrorCodeToString(int errorCode) {
    printf("(Code: %i): ", errorCode);
    switch (errorCode) {
        case shaderc_compilation_status_success: return "Success";
        case shaderc_compilation_status_invalid_stage: return "Failure To Deduce Shader Stage"; // error stage deduction
        case shaderc_compilation_status_compilation_error: return "Compilation Error";
        case shaderc_compilation_status_internal_error : return "Unexpected Failure"; // unexpected failure
        case shaderc_compilation_status_null_result_object: return "Null Result Objects";
        case shaderc_compilation_status_invalid_assembly: return "Invalid Assembly";
        case shaderc_compilation_status_validation_error: return "Validation Error";
        case shaderc_compilation_status_transformation_error : return "Transformation Error";
        case shaderc_compilation_status_configuration_error: return "Configuration Error";
        default: return "Unknown Error Code";
    }
}





typedef struct {
    float x,y,z;
} Float3;

Float3 newFloat3(const float x, const float y, const float z) {
    Float3 f = {.x = x, .y = y, .z = z};
    return f;
}


typedef struct {
    Float3 pos;
    Float3 col;
} vertex;

typedef struct {
    Float3 xyz;
} UBO;


void printQueueFlags(VkQueueFamilyProperties family) {
    if (family.queueFlags & VK_QUEUE_GRAPHICS_BIT) { printf(" Graphics "); }
    if (family.queueFlags & VK_QUEUE_SPARSE_BINDING_BIT) { printf(" Sparse Binding "); }
    if (family.queueFlags & VK_QUEUE_TRANSFER_BIT) { printf(" Transfer "); }
    if (family.queueFlags & VK_QUEUE_COMPUTE_BIT) { printf(" Compute "); }
}


int main() {

    VkInstance Instance = createInstance();


    GLFWwindow* Window = glfwCreateWindow(800, 600, "Application", VK_NULL_HANDLE, VK_NULL_HANDLE);
    VkSurfaceKHR Surface;
    glfwCreateWindowSurface(Instance, Window, VK_NULL_HANDLE, &Surface);

    VkPhysicalDevice PhysicalDevice = createPhysicalDevice(Instance);

    DeviceQueue GraphicsIndices = requestDeviceQueue(PhysicalDevice, VK_QUEUE_GRAPHICS_BIT, 1.0f);
    DeviceQueue TransferIndices = requestDeviceQueue(PhysicalDevice, VK_QUEUE_TRANSFER_BIT, 1.0f);

    VkDevice Device = createDevice(PhysicalDevice);

    VkQueue GraphicsQueue = getQueue(Device, GraphicsIndices);
    VkQueue TransferQueue = getQueue(Device, TransferIndices);


    U32 maxTransferBuffers = 5;
    VkCommandPool TransferCommandPool = createCommandPool(Device, TransferIndices.familyIndex, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    VkCommandBuffer* TransferBuffers = createCommandBuffers(Device, TransferCommandPool, maxTransferBuffers);

    VkFence* TransferFences = malloc(sizeof(VkFence) * maxTransferBuffers);
    for (int i = 0; i < maxTransferBuffers; i++) {
        VkFenceCreateInfo fenceInfo;
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.pNext = VK_NULL_HANDLE;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        vkCreateFence(Device, &fenceInfo, VK_NULL_HANDLE, &TransferFences[i]);
    }
    vkWaitForFences(Device, maxTransferBuffers, TransferFences, VK_TRUE, UINT64_MAX);
    vkResetFences(Device, maxTransferBuffers, TransferFences);


    VkBool32 graphicsQueueCanPresent = VK_FALSE;
    vkGetPhysicalDeviceSurfaceSupportKHR(PhysicalDevice, GraphicsIndices.familyIndex, Surface, &graphicsQueueCanPresent);
    if (graphicsQueueCanPresent != VK_TRUE) {
        printf("Queue Not Capable Of Framebuffer Presentation\n");
    }



    VkExtent2D SwapchainExtent;
    glfwGetWindowSize(Window, &SwapchainExtent.width, &SwapchainExtent.height);

    U32 backBuffers = 2;
    Swapchain Swapchain = createSwapchain(Device, PhysicalDevice, Surface, SwapchainExtent, GraphicsIndices.familyIndex, 1);
    FramebufferContainer Framebuffers = createFramebufferInfo(Device, &Swapchain);



    Float3 softBlue = newFloat3(0.1f,0.3f,1.0f);
    Float3 softGreen = newFloat3(1.0f,0.3f,0.1f);
    Float3 softRed = newFloat3(0.1f,1.0f,0.3f);


    // the greater the value of the z, the farther the vertex will be pushed
    U32 vertexCount = 6;
    U32 verticesPerShape = 3;
    vertex vertices[] = {
            {{0.0f, -0.5f, 0.0f}, {softBlue.x,softBlue.y,softBlue.z}},
            {{0.5f, 0.5f, 0.0f},{softGreen.x,softGreen.y,softGreen.z}},
            {{-0.5f, 0.5f, 0.0f},{softRed.x,softRed.y,softRed.z}},

            {{0.0f + 0.1f, -0.5f + 0.1f, 0.0f}, {softRed.x,softRed.y,softRed.z}},
            {{0.5f + 0.1f, 0.5f + 0.1f, 0.0f}, {softGreen.x,softGreen.y,softGreen.z}},
            {{-0.5f + 0.1f, 0.5f + 0.1f, 0.0f}, {softBlue.x,softBlue.y,softBlue.z}},
    };

// buffer


    VkCommandBuffer selectedVertexCopyBuffer;
    VkFence selectedVertexCopyFence;
    for (U32 i = 0; i < maxTransferBuffers; i++) {
        if (vkGetFenceStatus(Device, TransferFences[i]) == VK_NOT_READY) {
            selectedVertexCopyBuffer = TransferBuffers[i];
            selectedVertexCopyFence = TransferFences[i];
            break;
        }
    }

    Buffer VertexBuffer = createBuffer(Device,
                                       TransferIndices.familyIndex,
                                       TransferQueue,
                                       selectedVertexCopyBuffer,
                                       selectedVertexCopyFence, sizeof (vertex) * vertexCount,
                                       VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                                       vertices);



    // TODO: Get Current Working Directory, Then Find Shaders From There

    ShaderFile vertexShaderFile = createShader(Device, "D:\\Plutonium\\vertex.spv", "main");
    if (vertexShaderFile.result != VK_SUCCESS) printf("Shader Module Creation Result: %i\n", vertexShaderFile.result);

    ShaderFile fragmentShaderFile = createShader(Device, "D:\\Plutonium\\fragment.spv", "main");
    if (fragmentShaderFile.result != VK_SUCCESS) printf("Shader Module Creation Result: %i\n", fragmentShaderFile.result);



    U32 pipelineShaderStageCount = 2;
    VkPipelineShaderStageCreateInfo vertexShaderStage = createShaderStage(vertexShaderFile, VK_SHADER_STAGE_VERTEX_BIT);
    VkPipelineShaderStageCreateInfo fragmentShaderStage = createShaderStage(fragmentShaderFile, VK_SHADER_STAGE_FRAGMENT_BIT);
    VkPipelineShaderStageCreateInfo pipelineShaderStages[] = {vertexShaderStage, fragmentShaderStage};

// Vertex Input
    enum shaderType {
            shaderFloat = VK_FORMAT_R32_SFLOAT,
            shaderVec2 = VK_FORMAT_R32G32_SFLOAT,
            shaderVec3 = VK_FORMAT_R32G32B32_SFLOAT,
            shaderVec4 = VK_FORMAT_R32G32B32A32_SFLOAT,
    };

    U32 vertexDescriptionCount = 1;
    VkVertexInputBindingDescription vertexDescription;
    vertexDescription.binding = 0;
    vertexDescription.stride = sizeof(vertex);
    vertexDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    U32 vertexAttributeCount = 2;
    VkVertexInputAttributeDescription vertexAttribute[2];
    vertexAttribute[0].binding = 0;
    vertexAttribute[1].binding = 0;
    vertexAttribute[0].format = (VkFormat)shaderVec3;
    vertexAttribute[1].format = (VkFormat)shaderVec3;
    vertexAttribute[0].location = 0;
    vertexAttribute[1].location = 1;
    vertexAttribute[0].offset = offsetof(vertex, pos);
    vertexAttribute[1].offset = offsetof(vertex, col);


    VkPipelineVertexInputStateCreateInfo vertexInput;
    vertexInput.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInput.pNext = VK_NULL_HANDLE;
    vertexInput.flags = 0;
    vertexInput.vertexAttributeDescriptionCount = vertexAttributeCount;
    vertexInput.pVertexAttributeDescriptions = vertexAttribute;
    vertexInput.vertexBindingDescriptionCount = vertexDescriptionCount;
    vertexInput.pVertexBindingDescriptions = &vertexDescription;
//

    VkPipelineInputAssemblyStateCreateInfo inputAssembly = createInputAssemblyStage(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);

    VkViewport viewport = createViewport(Swapchain.extent);
    VkScissor scissor = createScissor(Swapchain.extent);
    VkPipelineViewportStateCreateInfo viewportState = createViewportState(viewport, scissor);





    U32 descriptorSetRequestCount = backBuffers;

    UniformBuffer* UniformBuffers = createUniformBuffers(Device, descriptorSetRequestCount, sizeof(UBO), TransferIndices.familyIndex);
    VkDescriptorBufferInfo* bufferInfo = malloc(sizeof(VkDescriptorBufferInfo) * descriptorSetRequestCount);

    // TODO: Descriptor Sets
/*
    VkDescriptorSetLayoutBinding uniformBinding = createNewBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, backBuffers, VK_SHADER_STAGE_VERTEX_BIT);
    VkDescriptorSetLayout layout = createDescriptorLayout(Device, 1, &uniformBinding);

    VkDescriptorPool descriptorPool = createDescriptorPool(Device, backBuffers, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);

    VkDescriptorSetLayout descriptorLayouts[] = {layout, layout};
    VkDescriptorSet* descriptorSets = createDescriptorSet(Device, backBuffers, descriptorLayouts, descriptorPool);


    writeDescriptor(Device, descriptorSets[0], VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, &bufferInfo[0], VK_NULL_HANDLE);
    writeDescriptor(Device, descriptorSets[1], VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, &bufferInfo[1], VK_NULL_HANDLE);
*/


    U32 descriptorBindingCount = 1;
    VkDescriptorSetLayoutBinding uniformBinding;
    uniformBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uniformBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    uniformBinding.descriptorCount = descriptorSetRequestCount;
    uniformBinding.binding = 0;
    uniformBinding.pImmutableSamplers = VK_NULL_HANDLE;

    VkDescriptorSetLayout layout;
    VkDescriptorSetLayoutCreateInfo descriptorLayoutInfo;
    descriptorLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptorLayoutInfo.pNext = VK_NULL_HANDLE;
    descriptorLayoutInfo.flags = 0;
    descriptorLayoutInfo.bindingCount = descriptorBindingCount;
    descriptorLayoutInfo.pBindings = &uniformBinding;

    vkCreateDescriptorSetLayout(Device, &descriptorLayoutInfo, VK_NULL_HANDLE, &layout);

    VkDescriptorPool descriptorPool;

    U32 descriptorPoolSizeCount = 1;
    VkDescriptorPoolSize descriptorPoolSize;
    descriptorPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorPoolSize.descriptorCount = descriptorSetRequestCount;

    VkDescriptorPoolCreateInfo poolInfo;
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.pNext = VK_NULL_HANDLE;
    poolInfo.flags = 0;
    poolInfo.maxSets = descriptorSetRequestCount;
    poolInfo.poolSizeCount = descriptorPoolSizeCount;
    poolInfo.pPoolSizes = &descriptorPoolSize;

    vkCreateDescriptorPool(Device, &poolInfo, VK_NULL_HANDLE, &descriptorPool);

    VkDescriptorSetLayout* descriptorLayoutArray = malloc(sizeof(VkDescriptorSetLayout) * descriptorSetRequestCount);
    for (U32 i = 0; i < descriptorSetRequestCount; i++) descriptorLayoutArray[i] = layout;

    VkDescriptorSetAllocateInfo descriptorAllocInfo;
    descriptorAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    descriptorAllocInfo.pNext = VK_NULL_HANDLE;
    descriptorAllocInfo.pSetLayouts = descriptorLayoutArray;
    descriptorAllocInfo.descriptorSetCount = descriptorSetRequestCount;
    descriptorAllocInfo.descriptorPool = descriptorPool;

    VkDescriptorSet* descriptorSets = malloc(sizeof(VkDescriptorSet) * descriptorSetRequestCount);
    vkAllocateDescriptorSets(Device, &descriptorAllocInfo, descriptorSets);

    free(descriptorLayoutArray);

    for (U32 i = 0; i < descriptorSetRequestCount; i++) {
        VkWriteDescriptorSet write;
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.pNext = VK_NULL_HANDLE;
        write.dstSet = descriptorSets[i];
        write.dstBinding = 0; // if this was an array it would not stay at zero (keep this in mind)
        write.dstArrayElement = 0;
        write.descriptorCount = 1;
        write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        write.pImageInfo = VK_NULL_HANDLE;
        write.pBufferInfo = &UniformBuffers[i].bufferInfo;
        write.pTexelBufferView = VK_NULL_HANDLE;

        vkUpdateDescriptorSets(Device, 1, &write, 0, VK_NULL_HANDLE);
    }


    VkPipelineLayout pipelineLayout  = createPipelineLayout(Device, 0, VK_NULL_HANDLE, 1, &layout);

    VkPipelineRasterizationStateCreateInfo rasterizer = createRasterizer(VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_CLOCKWISE, 1.0f);


// TODO: Add Multi Sample & Color Blend Simplification
    VkPipelineMultisampleStateCreateInfo multisample;
    multisample.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisample.pNext = VK_NULL_HANDLE;
    multisample.flags = 0;
    multisample.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisample.sampleShadingEnable = VK_FALSE;
    multisample.minSampleShading = 1.0f;
    multisample.pSampleMask = VK_NULL_HANDLE;
    multisample.alphaToCoverageEnable = VK_FALSE;
    multisample.alphaToOneEnable = VK_FALSE;


    U32 colorBlendAttachmentCount = 1;
    VkPipelineColorBlendAttachmentState colorBlendAttachment;
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional
//

    VkPipelineColorBlendStateCreateInfo colorBlend;
    colorBlend.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlend.pNext = VK_NULL_HANDLE;
    colorBlend.flags = 0;
    colorBlend.logicOpEnable = VK_FALSE;
    colorBlend.logicOp = VK_LOGIC_OP_COPY;
    colorBlend.attachmentCount = colorBlendAttachmentCount;
    colorBlend.pAttachments = &colorBlendAttachment;
    colorBlend.blendConstants[0] = 0.0f;
    colorBlend.blendConstants[1] = 0.0f;
    colorBlend.blendConstants[2] = 0.0f;
    colorBlend.blendConstants[3] = 0.0f;


    U32 renderPassAttachmentCount = 2;

    RenderPassAttachment colorAttachment = createRenderPassAttachment(VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                                       VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                       0,
                                       Swapchain.surfaceFormat.format,
                                       VK_SAMPLE_COUNT_1_BIT,
                                       VK_ATTACHMENT_LOAD_OP_CLEAR,
                                       VK_ATTACHMENT_STORE_OP_STORE,
                                       VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                                       VK_ATTACHMENT_STORE_OP_DONT_CARE);
    RenderPassAttachment depthAttachment = createRenderPassAttachment(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                                       VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                                       1,
                                       Swapchain.depthFormat,
                                       VK_SAMPLE_COUNT_1_BIT,
                                       VK_ATTACHMENT_LOAD_OP_CLEAR,
                                       VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                       VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                                       VK_ATTACHMENT_STORE_OP_DONT_CARE);

    VkAttachmentDescription descriptions[2] = {colorAttachment.description, depthAttachment.description};
    VkAttachmentReference references[2] = {colorAttachment.reference, depthAttachment.reference};



    U32 subpassCount = 1;
    VkSubpassDescription subpass;
    subpass.flags = 0;
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.inputAttachmentCount = 0;
    subpass.pInputAttachments = VK_NULL_HANDLE;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachment.reference;
    subpass.pResolveAttachments = VK_NULL_HANDLE;
    subpass.pDepthStencilAttachment = &depthAttachment.reference;
    subpass.preserveAttachmentCount = 0;
    subpass.pPreserveAttachments = VK_NULL_HANDLE;

    U32 dependencyCount = 1;
    VkSubpassDependency dependency;
    dependency.dependencyFlags = 0;
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcAccessMask = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;


    VkRenderPassCreateInfo renderPassInfo;
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.pNext = VK_NULL_HANDLE;
    renderPassInfo.flags = 0;
    renderPassInfo.attachmentCount = renderPassAttachmentCount;
    renderPassInfo.pAttachments = descriptions;
    renderPassInfo.subpassCount = subpassCount;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = dependencyCount;
    renderPassInfo.pDependencies = &dependency;


    VkRenderPass RenderPass;
    vkCreateRenderPass(Device, &renderPassInfo, VK_NULL_HANDLE, &RenderPass);

    VkPipelineDepthStencilStateCreateInfo depthStencil;
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.pNext = VK_NULL_HANDLE;
    depthStencil.flags = 0;
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.stencilTestEnable = VK_FALSE;
    VkStencilOpState frontStencilState;
    frontStencilState.failOp = VK_STENCIL_OP_ZERO;
    frontStencilState.passOp = VK_STENCIL_OP_ZERO;
    frontStencilState.depthFailOp = VK_STENCIL_OP_ZERO;
    frontStencilState.compareOp = 0;
    frontStencilState.compareMask = 0;
    frontStencilState.writeMask = 0;
    depthStencil.front = frontStencilState;
    VkStencilOpState backStencilState = frontStencilState;
    depthStencil.back = backStencilState;
    depthStencil.minDepthBounds = 0.0f;
    depthStencil.maxDepthBounds = 1.0f;

    VkGraphicsPipelineCreateInfo pipelineInfo;
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.pNext = VK_NULL_HANDLE;
    pipelineInfo.flags = 0;
    pipelineInfo.stageCount = pipelineShaderStageCount;
    pipelineInfo.pStages = pipelineShaderStages;
    pipelineInfo.pVertexInputState = &vertexInput;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pTessellationState = VK_NULL_HANDLE;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisample;
    pipelineInfo.pDepthStencilState = &depthStencil;
    pipelineInfo.pColorBlendState = &colorBlend;
    pipelineInfo.pDynamicState = VK_NULL_HANDLE;
    pipelineInfo.layout = pipelineLayout;
    pipelineInfo.renderPass = RenderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.basePipelineIndex = -1;


    VkPipeline GraphicsPipeline;
    vkCreateGraphicsPipelines(Device, VK_NULL_HANDLE, 1, &pipelineInfo, VK_NULL_HANDLE, &GraphicsPipeline);
    createFramebuffers(Device, RenderPass, Swapchain, &Framebuffers);


    VkCommandPool CommandPool = createCommandPool(Device, GraphicsIndices.familyIndex, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    VkCommandBuffer* CommandBuffers = createCommandBuffers(Device, CommandPool, Swapchain.swapchainImageCount);


    U32 frame = 0;

    VkSemaphoreCreateInfo semaphoreInfo;
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    semaphoreInfo.pNext = VK_NULL_HANDLE;
    semaphoreInfo.flags = 0;

    VkFenceCreateInfo fenceInfo;
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.pNext = VK_NULL_HANDLE;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    VkFence* renderFence = malloc(sizeof(VkFence) * backBuffers);
    VkSemaphore* waitSemaphores = malloc(sizeof(VkSemaphore) * backBuffers);
    VkSemaphore* signalSemaphores = malloc(sizeof(VkSemaphore) * backBuffers);

    for (U32 i = 0; i < backBuffers; i++) {
        vkCreateSemaphore(Device, &semaphoreInfo, VK_NULL_HANDLE, &waitSemaphores[i]);
        vkCreateSemaphore(Device, &semaphoreInfo, VK_NULL_HANDLE, &signalSemaphores[i]);
        vkCreateFence(Device, &fenceInfo, VK_NULL_HANDLE, &renderFence[i]);
    }



    U32 clearCount = 2;
    VkClearValue clears[2];

    clears[0].color.float32[0] = 0.02f;
    clears[0].color.float32[1] = 0.02f;
    clears[0].color.float32[2] = 0.02f;
    clears[0].color.float32[3] = 1.0f;

    clears[1].depthStencil.depth = 1.0f;
    clears[1].depthStencil.stencil = 0;

    while (!glfwWindowShouldClose(Window)) {
        glfwPollEvents();


        U32 imageIndex = 0;
        vkAcquireNextImageKHR(Device, Swapchain.swapchain, UINT64_MAX, waitSemaphores[frame], VK_NULL_HANDLE, &imageIndex);

        vkWaitForFences(Device, 1, &renderFence[frame], VK_TRUE, UINT64_MAX);
        vkResetFences(Device, 1, &renderFence[frame]);


        float value = ((float) sin((double)clock()/1000) / 2.0f) + 0.6f;
        UBO uniformData;
        uniformData.xyz = newFloat3(value, value, value);

        void* uniformUpload;
        vkMapMemory(Device, UniformBuffers[frame].memory, 0, sizeof(UBO), 0, &uniformUpload);
        memcpy(uniformUpload, &uniformData, sizeof(UBO));
        vkUnmapMemory(Device, UniformBuffers[frame].memory);


        beginFrameRecording(&CommandBuffers[imageIndex], RenderPass, Framebuffers.framebuffer[imageIndex], scissor, clearCount, clears);
        vkCmdBindPipeline(CommandBuffers[imageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, GraphicsPipeline);
        vkCmdBindDescriptorSets(CommandBuffers[imageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSets[frame], 0, VK_NULL_HANDLE);
        VkDeviceSize vertexBufferOffsets[] = {0};
        vkCmdBindVertexBuffers(CommandBuffers[imageIndex], 0, 1, &VertexBuffer.buffer, vertexBufferOffsets);
        vkCmdDraw(CommandBuffers[imageIndex], verticesPerShape, 1, 0, 0);
        endFrameRecording(&CommandBuffers[imageIndex]);




        VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

        VkSubmitInfo submitInfo;
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.pNext = VK_NULL_HANDLE;
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = &waitSemaphores[frame];
        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &CommandBuffers[imageIndex];
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = &signalSemaphores[frame];


        vkQueueSubmit(GraphicsQueue, 1, &submitInfo, renderFence[frame]);

        VkResult presentationResult;
        VkPresentInfoKHR presentInfo;
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.pNext = VK_NULL_HANDLE;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = &signalSemaphores[frame];
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = &Swapchain.swapchain;
        presentInfo.pImageIndices = &imageIndex;
        presentInfo.pResults = &presentationResult;

        vkQueuePresentKHR(GraphicsQueue, &presentInfo);


        glfwSwapBuffers(Window);
        frame = (frame+1)%backBuffers;
    }

    vkDeviceWaitIdle(Device);


    destroyBuffer(Device, &VertexBuffer);


    vkDestroyDescriptorPool(Device, descriptorPool, VK_NULL_HANDLE);
    vkDestroyDescriptorSetLayout(Device, layout, VK_NULL_HANDLE);
    for (U32 i = 0; i < descriptorSetRequestCount; i++) {
        destroyBuffer(Device, &UniformBuffers[i]);
    }


    for (int i = 0; i < backBuffers; i++) {
        vkDestroySemaphore(Device, waitSemaphores[i], VK_NULL_HANDLE);
        vkDestroySemaphore(Device, signalSemaphores[i], VK_NULL_HANDLE);
        vkDestroyFence(Device, renderFence[i], VK_NULL_HANDLE);
    }

    destroyFramebufferContainer(Device, Swapchain, &Framebuffers);

    vkFreeCommandBuffers(Device, CommandPool, Swapchain.swapchainImageCount, CommandBuffers);
    vkDestroyCommandPool(Device, CommandPool, VK_NULL_HANDLE);






    vkDestroyPipeline(Device, GraphicsPipeline, VK_NULL_HANDLE);

    vkDestroyRenderPass(Device, RenderPass, VK_NULL_HANDLE);

    vkDestroyPipelineLayout(Device, pipelineLayout, VK_NULL_HANDLE);


    destroyShader(Device, &vertexShaderFile);
    destroyShader(Device, &fragmentShaderFile);


    destroySwapchain(Device, &Swapchain);


    for (U32 i = 0; i < maxTransferBuffers; i++)
        vkDestroyFence(Device, TransferFences[i], VK_NULL_HANDLE);
    vkFreeCommandBuffers(Device, TransferCommandPool, maxTransferBuffers, TransferBuffers);
    vkDestroyCommandPool(Device, TransferCommandPool, VK_NULL_HANDLE);

    vkDestroyDevice(Device, VK_NULL_HANDLE);


    vkDestroySurfaceKHR(Instance, Surface, VK_NULL_HANDLE);

    destroyInstance(&Instance);

    return 0;
}

