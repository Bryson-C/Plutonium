//
// Created by Owner on 5/21/2022.
//

#ifndef PLUTONIUM_ABSTRACTIONS_H
#define PLUTONIUM_ABSTRACTIONS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "vulkan/vulkan.h"
#include "glfw3.h"

typedef uint32_t U32;
#define PL_Debug 1

// Instance
VkInstance createInstance();

void destroyInstance(VkInstance* instance);

// Device

struct DeviceQueue_t { U32 familyIndex, queueIndex; };
typedef struct DeviceQueue_t DeviceQueue;

VkPhysicalDevice createPhysicalDevice(VkInstance instance);
VkDevice createDevice(VkPhysicalDevice physicalDevice);
DeviceQueue requestDeviceQueue(VkPhysicalDevice physicalDevice, VkQueueFlagBits flags, float priority);
VkQueue getQueue(VkDevice device, DeviceQueue queue);

void destroyDevice();

// Pools

VkCommandPool createCommandPool(VkDevice device, U32 queueFamily, U32 flags);
VkCommandBuffer* createCommandBuffers(VkDevice device, VkCommandPool pool, U32 count);

// Buffers

struct Buffer_t { VkBuffer buffer; VkDeviceMemory memory; };
typedef struct Buffer_t Buffer;

Buffer createBuffer(VkDevice device, U32 queueFamily,
                    VkQueue queue, VkCommandBuffer commandBuffer,
                    VkFence fence, VkDeviceSize size,
                    VkBufferUsageFlagBits usageFlags, void* data);

void destroyBuffer(VkDevice device, Buffer* buffer);

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

Swapchain createSwapchain(VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkExtent2D extent, U32 queueFamily, U32 backBufferCount);
FramebufferContainer createFramebufferInfo(VkDevice device, Swapchain* swapchain);
void createFramebuffers(VkDevice device, VkRenderPass renderPass, Swapchain swapchain, FramebufferContainer* framebufferContainer);

void destroySwapchain(VkDevice device, Swapchain* swapchain);
void destroyFramebufferContainer(VkDevice device, Swapchain swapchain, FramebufferContainer* framebufferContainers);

// Pipeline

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

ShaderFile createShader(VkDevice device, const char* path, const char* entryPoint);

VkPipelineShaderStageCreateInfo createShaderStage(ShaderFile shader, VkShaderStageFlagBits shaderStage);
VkPipelineInputAssemblyStateCreateInfo createInputAssemblyStage(VkPrimitiveTopology topology);

VkPipelineViewportStateCreateInfo createViewportState(VkViewport viewport, VkScissor scissor);
VkViewport createViewport(VkExtent2D extent);
VkScissor createScissor(VkExtent2D extent);

VkPipelineLayout createPipelineLayout(VkDevice device,
                                      U32 pushConstantCount, VkPushConstantRange* pushConstants,
                                      U32 setLayoutCount, VkDescriptorSetLayout* setLayouts);
VkPipelineRasterizationStateCreateInfo createRasterizer(VkPolygonMode polygonMode,
                                                        VkCullModeFlagBits cullMode,
                                                        VkFrontFace frontFace,
                                                        float lineWidth);


void destroyShader(VkDevice device, ShaderFile* shader);

// Render Pass

struct RenderPassAttachment_t {
    VkAttachmentDescription description;
    VkAttachmentReference reference;
};
typedef struct RenderPassAttachment_t RenderPassAttachment;

RenderPassAttachment createRenderPassAttachment(VkImageLayout finalLayout, VkImageLayout imageType, U32 attachment, VkFormat format,
                                                VkSampleCountFlags samples, VkAttachmentLoadOp loadOp, VkAttachmentStoreOp storeOp,
                                                VkAttachmentLoadOp stencilLoadOp, VkAttachmentStoreOp stencilStoreOp);




// TODO: ShaderC


#endif //PLUTONIUM_ABSTRACTIONS_H
