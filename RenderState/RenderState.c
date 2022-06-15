//
// Created by Owner on 6/5/2022.
//


#include "RenderState.h"




static U32 shaderCount = 0;
static VRSShaderInfo shaderInfo[MAX_SHADERS];

void ApplyShader(VkShaderStageFlagBits stage, const char* path, const char* entryPoint) { shaderInfo[shaderCount++] = (VRSShaderInfo){path, entryPoint, stage}; }

static VkPipelineVertexInputStateCreateInfo vertexInput;
void ApplyVertexInput(U32 attributeCount, VkVertexInputAttributeDescription* attributes, U32 bindingCount, VkVertexInputBindingDescription* bindings) {
    vertexInput = createVertexInput(attributeCount, attributes, bindingCount, bindings);
}

static U32 pushConstantCount = 0;
static VkPushConstantRange* pushConstants;
static U32 descriptorLayoutCount = 0;
static VkDescriptorSetLayout* descriptorLayouts;

void ApplyPushConstants(U32 count, VkPushConstantRange* ranges) {
    pushConstantCount = count;
    pushConstants = ranges;
}

void ApplyDescriptorLayouts(U32 count, VkDescriptorSetLayout* layouts) {
    descriptorLayoutCount = count;
    descriptorLayouts = layouts;
}


void CreateRenderStateRenderer() {

    glfwCreateWindowSurface(VRS.instance, VRS.window, VK_NULL_HANDLE, &VRS.surface);
    VRS.extent = getSurfaceExtent(VRS.window);

    if (VRS.extent.width <= 0 || VRS.extent.height <= 0 ) glfwWaitEvents();


    VkBool32 supported;
    vkGetPhysicalDeviceSurfaceSupportKHR(VRS.physicalDevice, VRS.graphicsQueue.familyIndex, VRS.surface, &supported);
    if (!supported) { /* Uh oh */ }

    VkPipelineShaderStageCreateInfo* shaderStages = malloc(sizeof(VkPipelineShaderStageCreateInfo) * shaderCount);
    for (U32 i = 0; i < shaderCount; i++) {
        ShaderFile info = createShader(VRS.device, shaderInfo[i].path, shaderInfo[i].entryPoint);
        shaderStages[i] = createShaderStage(info, shaderInfo[i].stage);
    }


    VRS.swapchain = createSwapchain(VRS.device, VRS.physicalDevice, VRS.surface, VRS.extent, VRS.graphicsQueue.familyIndex, 1);
    VRS.framebuffers = createFramebufferInfo(VRS.device, &VRS.swapchain);

    VRS.graphicsPool = createCommandPool(VRS.device, VRS.graphicsQueue.familyIndex, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    VRS.renderBuffers = createCommandBuffers(VRS.device, VRS.graphicsPool, VRS.swapchain.swapchainImageCount);

    VRS.renderPass = createRenderPass(VRS.device, VRS.swapchain);
    VRS.pipelineLayout = createPipelineLayout(VRS.device, pushConstantCount, pushConstants, descriptorLayoutCount, descriptorLayouts);

    PipelineBuilder builder = newPipelineBuilder();
    addRenderPassToPipelineBuilder(&builder, VRS.renderPass);
    addShadersToPipelineBuilder(&builder, shaderCount, shaderStages);
    addExtent2dToPipelineBuilder(&builder, VRS.extent);
    addVertexInputToPipelineBuilder(&builder, vertexInput);
    addPipelineLayoutToPipelineBuilder(&builder, VRS.pipelineLayout);

    VRS.graphicsPipeline = createPipelineFromBuilder(VRS.device, VRS.swapchain, &builder);

    createFramebuffers(VRS.device, VRS.renderPass.renderPass, VRS.swapchain, &VRS.framebuffers);
    VRS.renderFences = createFences(VRS.device, VRS.backBuffers);
    VRS.waitSemaphore = createSemaphores(VRS.device, VRS.backBuffers);
    VRS.signalSemaphore = createSemaphores(VRS.device, VRS.backBuffers);
}



void CreateRenderState() {
    VRS.backBuffers = 2;
    VRS.activeFrame = 0;

    VRS.instance = createInstance();
    VRS.physicalDevice = createPhysicalDevice(VRS.instance);

    VRS.graphicsQueue = requestDeviceQueue(VRS.physicalDevice, VK_QUEUE_GRAPHICS_BIT, 1.0f);
    VRS.transferQueue = requestDeviceQueue(VRS.physicalDevice, VK_QUEUE_TRANSFER_BIT, 1.0f);

    VRS.device = createDevice(VRS.physicalDevice);

    getQueue(VRS.device, &VRS.graphicsQueue);


    getQueue(VRS.device, &VRS.transferQueue);

    VRS.transferPool = createCommandPool(VRS.device, VRS.graphicsQueue.familyIndex, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    VRS.transferBuffers = createCommandBuffers(VRS.device, VRS.transferPool, MAX_TRANSFER_CMD_BUFFERS);
    VRS.transferFences = createFences(VRS.device, MAX_TRANSFER_CMD_BUFFERS);

    VRS.window = glfwCreateWindow(800, 600, "Application", VK_NULL_HANDLE, VK_NULL_HANDLE);

    CreateRenderStateRenderer();
}

void BeginDraw() {
    VkResult imageAcquireResult = vkAcquireNextImageKHR(VRS.device, VRS.swapchain.swapchain, UINT64_MAX, VRS.waitSemaphore[VRS.activeFrame], VK_NULL_HANDLE, &VRS.imageIndex);
    if (imageAcquireResult == VK_ERROR_OUT_OF_DATE_KHR) { // TODO: Make Resizing Code Smaller + Actually Working
        DestroyRenderStateRenderer();
        CreateRenderStateRenderer();
        return;
    }

    vkWaitForFences(VRS.device, 1, &VRS.renderFences[VRS.activeFrame], VK_TRUE, UINT64_MAX);
    vkResetFences(VRS.device, 1, &VRS.renderFences[VRS.activeFrame]);


    VkScissor scissor = createScissor(VRS.extent);

    beginFrameRecording(&VRS.renderBuffers[VRS.imageIndex], VRS.renderPass, VRS.framebuffers.framebuffer[VRS.imageIndex], scissor);
    vkCmdBindPipeline(VRS.renderBuffers[VRS.imageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, VRS.graphicsPipeline);
}

void SubmitDraw() {

    endFrameRecording(&VRS.renderBuffers[VRS.imageIndex]);

    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

    VkSubmitInfo submitInfo;
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pNext = VK_NULL_HANDLE;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &VRS.waitSemaphore[VRS.activeFrame];
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &VRS.renderBuffers[VRS.imageIndex];
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &VRS.signalSemaphore[VRS.activeFrame];


    vkQueueSubmit(VRS.graphicsQueue.queue, 1, &submitInfo, VRS.renderFences[VRS.activeFrame]);

    VkResult presentationResult;
    VkPresentInfoKHR presentInfo;
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.pNext = VK_NULL_HANDLE;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &VRS.signalSemaphore[VRS.activeFrame];
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &VRS.swapchain.swapchain;
    presentInfo.pImageIndices = &VRS.imageIndex;
    presentInfo.pResults = &presentationResult;

    VkResult renderResult = vkQueuePresentKHR(VRS.graphicsQueue.queue, &presentInfo);
    if (renderResult == VK_ERROR_OUT_OF_DATE_KHR || renderResult == VK_SUBOPTIMAL_KHR) {
        DestroyRenderStateRenderer();
        CreateRenderStateRenderer();
        return;
    }

    VRS.activeFrame = (VRS.activeFrame+1)%VRS.backBuffers;
}

void DestroyRenderStateRenderer() {

    vkDeviceWaitIdle(VRS.device);
    for (U32 i = 0; i < VRS.backBuffers; i++) {
        vkDestroyFence(VRS.device, VRS.renderFences[i], VK_NULL_HANDLE);
        vkDestroySemaphore(VRS.device, VRS.waitSemaphore[i], VK_NULL_HANDLE);
        vkDestroySemaphore(VRS.device, VRS.signalSemaphore[i], VK_NULL_HANDLE);
    }

    vkDestroyPipeline(VRS.device, VRS.graphicsPipeline, VK_NULL_HANDLE);
    vkDestroyPipelineLayout(VRS.device, VRS.pipelineLayout, VK_NULL_HANDLE);
    destroyFramebufferContainer(VRS.device, VRS.swapchain, &VRS.framebuffers);
    vkDestroyRenderPass(VRS.device, VRS.renderPass.renderPass, VK_NULL_HANDLE);
    vkDestroyCommandPool(VRS.device, VRS.graphicsPool, VK_NULL_HANDLE);
    destroySwapchain(VRS.device, &VRS.swapchain);
    vkDestroySurfaceKHR(VRS.instance, VRS.surface, VK_NULL_HANDLE);

    VRS.activeFrame = 0;
}
void DestroyRenderState() {
    DestroyRenderStateRenderer();
    // destroy the rest of the objects
}

typedef struct {
    VkCommandBuffer buffer;
    VkFence fence;
} BufferAndFence;

BufferAndFence GetFreeTransferFence() {
    for (int i = 0; i < MAX_TRANSFER_CMD_BUFFERS; i++) {
        if (vkGetFenceStatus(VRS.device, VRS.transferFences[i]) == VK_NOT_READY) {
            return (BufferAndFence){VRS.transferBuffers[i], VRS.transferFences[i]};
        }
    }
    vkWaitForFences(VRS.device, 1, &VRS.transferFences[0], VK_TRUE, UINT64_MAX);
    vkResetFences(VRS.device, 1, &VRS.transferFences[0]);
    return (BufferAndFence){VRS.transferBuffers[0], VRS.transferFences[0]};
}


Buffer RequestBuffer(VkDeviceSize size, VkBufferUsageFlagBits usage, void* data, bool useStaging) {
    BufferAndFence selectedBuffer = GetFreeTransferFence();
    DeviceQueue queue = (useStaging) ? VRS.transferQueue : VRS.graphicsQueue;
    if (useStaging) {
        return createBuffer(VRS.device, queue,
                            selectedBuffer.buffer, selectedBuffer.fence,
                            size, usage, data);
    }
    return createBufferWithoutStaging(VRS.device, size, queue, usage, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
}






