//
// Created by Owner on 5/21/2022.
//

#ifndef PLUTONIUM_ABSTRACTIONS_H
#define PLUTONIUM_ABSTRACTIONS_H

// TODO: Clean Up Code
// TODO: Make Code PascalCase
// TODO: Mark Engine Only Code (aka Code Users Should Not Interact With)
// TODO: Add Documentation

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

#include "../lib/stb_image.h"
#include "vulkan/vulkan.h"
#include "glfw3.h"

#include "Globals.h"
#include "Buffers.h"
#include "Descriptors.h"


typedef uint32_t U32;
typedef int32_t I32;
#define PL_Debug 1

// Others

inline I32 RandomInt(I32 from, I32 to)
{
    srand(clock());
    return rand()%to+from;
}
inline float RandomFloat(int from, int to)
{
    srand(clock());
    return (float)(rand()%to+from);
}

VkFence* createFences(VkDevice device, U32 count);
VkSemaphore* createSemaphores(VkDevice device, U32 count);

void destroyFences(VkDevice device, U32 fenceCount, VkFence* fences);
void destroySemaphores(VkDevice device, U32 semaphoreCount, VkSemaphore* semaphores);

uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);


// Instance
VkInstance createInstance();

void destroyInstance(VkInstance* instance);

// Device

struct DeviceQueue_t { U32 familyIndex, queueIndex; VkQueue queue; };
typedef struct DeviceQueue_t DeviceQueue;

VkPhysicalDevice createPhysicalDevice(VkInstance instance);
VkPhysicalDeviceProperties GetPhysicalDeviceProperties();

VkDevice createDevice(VkPhysicalDevice physicalDevice);
DeviceQueue requestDeviceQueue(VkPhysicalDevice physicalDevice, VkQueueFlagBits flags, float priority);
void getQueue(VkDevice device, DeviceQueue* queue);

void destroyDevice();

// Pools

VkCommandPool createCommandPool(VkDevice device, U32 queueFamily, U32 flags);
VkCommandBuffer* createCommandBuffers(VkDevice device, VkCommandPool pool, U32 count);



// Swapchain

struct Swapchain_t {
    VkSwapchainKHR swapchain;
    VkPresentModeKHR presentMode;
    VkSurfaceFormatKHR surfaceFormat;
    VkExtent2D extent;
    U32 minImageCount;
    U32 swapchainImageCount;

    U32 depthBuffers;
    VkImage* depthImage;
    VkImageView* depthView;
    VkDeviceMemory* depthMemory;
    VkFormat depthFormat;

};
typedef struct Swapchain_t Swapchain;

struct FramebufferContainer_t {
    U32 imageCount;
    VkImage* image;
    VkImageView* imageView;
    VkFramebuffer* framebuffer;
};
typedef struct FramebufferContainer_t FramebufferContainer;

VkExtent2D getSurfaceExtent(GLFWwindow* window);

Swapchain createSwapchain(VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkExtent2D extent, U32 queueFamily, U32 depthBufferCount);
FramebufferContainer createFramebufferInfo(VkDevice device, Swapchain* swapchain);
void createFramebuffers(VkDevice device, VkRenderPass renderPass, Swapchain swapchain, FramebufferContainer* framebufferContainer);

void destroySwapchain(VkDevice device, Swapchain* swapchain);
void destroyFramebufferContainer(VkDevice device, Swapchain swapchain, FramebufferContainer* framebufferContainers);

// Pipeline

// Render Pass

struct RenderPassAttachment_t {
    VkAttachmentDescription description;
    VkAttachmentReference reference;
};
typedef struct RenderPassAttachment_t RenderPassAttachment;

struct RenderPass_t {
    U32 subpassCount, dependencyCount, attachmentCount, clearCount;
    VkAttachmentDescription* descriptions;
    VkAttachmentReference* references;
    VkSubpassDescription subpass;
    VkSubpassDependency dependency;
    VkRenderPass renderPass;
    VkClearValue* clears;
};
typedef struct RenderPass_t RenderPass;
RenderPass createRenderPass(VkDevice device, Swapchain swapchain);

RenderPassAttachment createRenderPassAttachment(VkImageLayout finalLayout, VkImageLayout imageType, U32 attachment, VkFormat format,
                                                VkSampleCountFlags samples, VkAttachmentLoadOp loadOp, VkAttachmentStoreOp storeOp,
                                                VkAttachmentLoadOp stencilLoadOp, VkAttachmentStoreOp stencilStoreOp);

struct ShaderFile_t {
    FILE* file;
    char* buffer;
    size_t size;
    const char* entryPoint;
    const char* path;
    VkShaderModule module;
    VkResult result;
};
typedef struct ShaderFile_t ShaderFile;
typedef struct VkRect2D VkScissor;

struct PipelineBuilder_t {
    I32 hasShaders;
    U32 shaderStageCount;
    VkPipelineShaderStageCreateInfo* shaderStages;

    I32 hasVertexInput;
    VkPipelineVertexInputStateCreateInfo vertexInput;

    I32 hasInputAssembly;
    VkPipelineInputAssemblyStateCreateInfo inputAssembly;

    I32 hasViewport, hasScissor, hasExtent;
    VkViewport viewport; // only needed if an invalid viewport state exists
    VkScissor scissor; // only needed if an invalid viewport state exists
    VkExtent2D extent; // only needed if no scissor or viewport are valid

    I32 hasViewportState;
    VkPipelineViewportStateCreateInfo viewportState;

    I32 hasRasterizer;
    VkPipelineRasterizationStateCreateInfo rasterizer;

    I32 hasMultisample;
    VkPipelineMultisampleStateCreateInfo multisample;

    I32 hasDepthStencil;
    VkPipelineDepthStencilStateCreateInfo depthStencil;

    I32 hasColorBlend;
    VkPipelineColorBlendStateCreateInfo colorBlend;

    I32 hasPipelineLayout;
    VkPipelineLayout pipelineLayout;

    I32 hasRenderPass;
    RenderPass renderPass;
    U32 subpass;
};
typedef struct PipelineBuilder_t PipelineBuilder;
PipelineBuilder newPipelineBuilder();

void addShadersToPipelineBuilder(PipelineBuilder* builder, U32 shaderCount, VkPipelineShaderStageCreateInfo* shaders);
void addVertexInputToPipelineBuilder(PipelineBuilder* builder, VkPipelineVertexInputStateCreateInfo vertexInput);
void addInputAssemblyToPipelineBuilder(PipelineBuilder* builder, VkPipelineInputAssemblyStateCreateInfo inputAssembly);
void addExtent2dToPipelineBuilder(PipelineBuilder* builder, VkExtent2D extent);
void addViewportStateToPipelineBuilder(PipelineBuilder* builder, VkPipelineViewportStateCreateInfo viewportState);
void addPipelineLayoutToPipelineBuilder(PipelineBuilder* builder, VkPipelineLayout pipelineLayout);
void addRasterizerToPipelineBuilder(PipelineBuilder* builder, VkPipelineRasterizationStateCreateInfo rasterizer);
void addMultisampleStateToPipelineBuilder(PipelineBuilder* builder, VkPipelineMultisampleStateCreateInfo multisample);
void addColorBlendStateToPipelineBuilder(PipelineBuilder* builder, VkPipelineColorBlendStateCreateInfo colorBlend);
void addRenderPassToPipelineBuilder(PipelineBuilder* builder, RenderPass renderPass);
void addDepthStencilToPipelineBuilder(PipelineBuilder* builder, VkPipelineDepthStencilStateCreateInfo depthStencil);

VkPipeline createPipelineFromBuilder(VkDevice device, Swapchain swapchain, PipelineBuilder* builder);




ShaderFile createShader(VkDevice device, const char* path, const char* entryPoint);
VkPipelineShaderStageCreateInfo createShaderStage(ShaderFile shader, VkShaderStageFlagBits shaderStage);
VkVertexInputAttributeDescription createVertexAttribute(U32 binding, U32 location, VkFormat format, U32 offset);
VkVertexInputBindingDescription createVertexBinding(U32 binding, VkVertexInputRate inputRate, VkDeviceSize stride);
VkPipelineVertexInputStateCreateInfo createVertexInput(U32 attributeCount, VkVertexInputAttributeDescription* attributes, U32 bindingCount, VkVertexInputBindingDescription* bindings);
VkPipelineInputAssemblyStateCreateInfo createInputAssemblyStage(VkPrimitiveTopology topology);
VkViewport createViewport(VkExtent2D extent);
VkScissor createScissor(VkExtent2D extent);
VkPipelineViewportStateCreateInfo createViewportState(VkViewport viewport, VkScissor scissor);

VkPipelineLayout createPipelineLayout(VkDevice device,
                                      U32 pushConstantCount, VkPushConstantRange* pushConstants,
                                      U32 setLayoutCount, VkDescriptorSetLayout* setLayouts);
VkPipelineRasterizationStateCreateInfo createRasterizer(VkPolygonMode polygonMode,
                                                        VkCullModeFlagBits cullMode,
                                                        VkFrontFace frontFace,
                                                        float lineWidth);

VkPipelineMultisampleStateCreateInfo createMultisampleState();
VkPipelineColorBlendAttachmentState createColorBlendAttachment();
VkPipelineColorBlendStateCreateInfo createColorBlend(U32 attachmentCount, VkPipelineColorBlendAttachmentState attachment);
VkPipelineDepthStencilStateCreateInfo createDepthStencilState();


void destroyShader(VkDevice device, ShaderFile* shader);

// Descriptors




// TODO: ShaderC

// Drawing


void beginFrameRecording(VkCommandBuffer* buffer, RenderPass renderPass, VkFramebuffer framebuffer, VkScissor scissor);
void endFrameRecording(VkCommandBuffer* buffer);


typedef struct {
    VkImage image;
    VkImageView view;
    VkDeviceMemory memory;
    VkDeviceSize requiredSize;
} Image;

typedef struct {
    Image image;
    VkDescriptorSet set;
    int32_t index;
} Texture;


Image createImage(VkDevice device, VkImageType type, VkFormat format, VkExtent3D extent, VkImageUsageFlagBits usage, DeviceQueue queue);
void destroyImage(VkDevice device, Image image);

Texture CreateTexture(VkDevice device, DeviceQueue queue, VkDescriptorSet set, U32 setBinding, const char* path);
void DestroyTexture(VkDevice device, Texture* texture);


VkSampler createSampler(VkDevice device, VkFilter filter, VkSamplerAddressMode addressMode);
void destroySampler(VkDevice device, VkSampler sampler);

#endif //PLUTONIUM_ABSTRACTIONS_H
