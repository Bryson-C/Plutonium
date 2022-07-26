//
// Created by Owner on 6/23/2022.
//

#ifndef PLUTONIUM_PLUTONIUMCORE_H
#define PLUTONIUM_PLUTONIUMCORE_H

// Plutonium core? We should open that with a screwdriver

#include <stdio.h>              // required: printf, fprintf
#include <stdlib.h>             // required: malloc, realloc, calloc
#include <assert.h>             // required: assert

#include <vulkan/vulkan.h>      // required: much functionality
#include <glfw3.h>              // required: much functionality


typedef struct {
    VkInstance instance;
    VkDebugUtilsMessengerEXT priv_Messenger;
} PLCore_Instance;
typedef struct {
    VkQueue queue;
    uint32_t queueIndex;
    uint32_t familyIndex;
    VkQueueFlagBits flag;
} PLCore_DeviceQueue;
typedef struct {
    VkPhysicalDevice physicalDevice;
    VkPhysicalDeviceProperties properties;
    VkPhysicalDeviceFeatures features;
    VkPhysicalDeviceMemoryProperties memoryProperties;

    PLCore_DeviceQueue graphicsQueue;
    PLCore_DeviceQueue transferQueue;

    VkDevice device;
} PLCore_Device;
typedef struct {
    VkCommandPool pool;                 // Buffer Allocator
    VkCommandBuffer* buffers;           // Buffer Object Array
} PLCore_CommandPool;
typedef struct {
    PLCore_Instance pl_instance;
    PLCore_Device pl_device;

    PLCore_CommandPool transferPool;
} PLCore_RenderInstance;
typedef struct {
    VkSwapchainKHR swapchain;           // Framebuffer Organiser
    VkPresentModeKHR presentMode;       // How The Framebuffers Are Presented
    VkSurfaceFormatKHR surfaceFormat;   // The Format And Color Format That Is Used For Rendering
    VkFormat depthFormat;               // Format Of The Depth Buffer For The Render Pass
    VkExtent2D resolution;              // X,Y Dimensions Of The Screen
} PLCore_Swapchain;
typedef struct {
    GLFWwindow* window;
    VkSurfaceKHR surface;
    VkExtent2D resolution;
    VkViewport viewport;
    VkRect2D renderArea;
} PLCore_Window;
typedef struct {
    VkImage image;
    VkImageView view;
    VkDeviceMemory memory;
} PLCore_Image;
typedef struct {
    VkSwapchainKHR swapchain;
    VkImage* swapchainImages;
    VkImageView* swapchainImageViews;
    uint32_t swapchainImageCount;
    PLCore_Image pl_depthImage;

    VkRenderPass renderPass;

    VkFramebuffer* framebuffers;
    PLCore_CommandPool graphicsPool;


} PLCore_Renderer;
typedef struct {
    FILE* file;
    char* buffer;
    size_t size;
    const char* entryPoint;
    const char* path;
    VkShaderModule module;
    VkResult result;
} PLCore_ShaderModule;
typedef struct {
    int32_t hasShaders;
    uint32_t shaderStageCount;
    VkPipelineShaderStageCreateInfo* shaderStages;

    int32_t hasVertexInput;
    VkPipelineVertexInputStateCreateInfo vertexInput;

    int32_t hasInputAssembly;
    VkPipelineInputAssemblyStateCreateInfo inputAssembly;

    int32_t hasViewport, hasScissor, hasExtent;
    VkViewport viewport; // only needed if an invalid viewport state exists
    VkRect2D scissor; // only needed if an invalid viewport state exists
    VkExtent2D extent; // only needed if no scissor or viewport are valid

    int32_t hasViewportState;
    VkPipelineViewportStateCreateInfo viewportState;

    int32_t hasRasterizer;
    VkPipelineRasterizationStateCreateInfo rasterizer;

    int32_t hasMultisample;
    VkPipelineMultisampleStateCreateInfo multisample;

    int32_t hasDepthStencil;
    VkPipelineDepthStencilStateCreateInfo depthStencil;

    int32_t hasColorBlend;
    VkPipelineColorBlendStateCreateInfo colorBlend;

    int32_t hasPipelineLayout;
    VkPipelineLayout pipelineLayout;
} PLCore_PipelineBuilder;
typedef struct {
    VkPipeline pipeline;
    VkPipelineLayout layout;
} PLCore_GraphicsPipeline;

VkInstance          PLCore_Priv_CreateInstance(VkDebugUtilsMessengerEXT* messenger);
VkPhysicalDevice    PLCore_Priv_CreatePhysicalDevice(VkInstance instance, uint32_t* queueFamilyCount, VkQueueFamilyProperties** queueFamilies);
VkDevice            PLCore_Priv_CreateDevice(VkPhysicalDevice physicalDevice, uint32_t queueFamilyCount, VkQueueFamilyProperties* queueFamilyProperties, VkPhysicalDeviceFeatures features, uint32_t queueRequestCount, VkQueueFlagBits* queueRequest, PLCore_DeviceQueue** queues);
GLFWwindow*         PLCore_Priv_CreateWindow(VkInstance instance, uint32_t width, uint32_t height, VkSurfaceKHR* surface);
VkRenderPass        PLCore_Priv_CreateRenderPass(VkDevice device, VkFormat presentableFormat, VkFormat depthFormat);
VkCommandPool       PLCore_Priv_CreateCommandPool(VkDevice device, uint32_t queueFamily, VkCommandPoolCreateFlagBits flags);
VkCommandBuffer*    PLCore_Priv_CreateCommandBuffers(VkDevice device, VkCommandPool pool, uint32_t count);
VkSwapchainKHR      PLCore_Priv_CreateSwapchain(VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkExtent2D screenResolution, uint32_t queueFamily, VkSurfaceFormatKHR* surfaceFormat, VkPresentModeKHR* presentMode);
VkImage*            PLCore_Priv_AcquireSwapchainImages(VkDevice device, VkSwapchainKHR swapchain, VkFormat renderFormat, VkImageView** imageViews, uint32_t* imageCount);
VkImage             PLCore_Priv_CreateDepthBuffer(VkDevice device, VkPhysicalDeviceMemoryProperties memoryProperties, VkImageView* depthView, VkFormat depthFormat, VkExtent2D resolution, uint32_t queueFamily);
VkFramebuffer       PLCore_Priv_CreateFramebuffer(VkDevice device, VkExtent2D resolution, VkRenderPass renderPass, VkImageView swapchainImage, VkImageView depthView);

PLCore_RenderInstance   PLCore_CreateRenderingInstance();
PLCore_Window           PLCore_CreateWindow(VkInstance instance, uint32_t width, uint32_t height);
PLCore_Renderer         PLCore_CreateRenderer(PLCore_RenderInstance instance, PLCore_Window window);


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




#endif //PLUTONIUM_PLUTONIUMCORE_H
