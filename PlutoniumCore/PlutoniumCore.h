//
// Created by Owner on 6/23/2022.
//

#ifndef PLUTONIUM_PLUTONIUMCORE_H
#define PLUTONIUM_PLUTONIUMCORE_H

// Plutonium core? We should open that with a screwdriver
// TODO: Split Code Into Seperate Files

#include <stdio.h>              // required: printf, fprintf
#include <stdlib.h>             // required: malloc, realloc, calloc
#include <string.h>             // required: memcpy
#include <assert.h>             // required: assert
#include <time.h>               // required: clock(), clock_t

#include <vulkan/vulkan.h>      // required: much functionality
#include <glfw3.h>              // required: much functionality

#include <cglm/cglm.h>          // required: matrix and vectorFunctionality
#include <cglm/struct.h>        // required: extensions of base 'cglm/cglm.h' functionality

#include "SpirvReflection.h"

// no use, i just thought it was funny
#define verylong long long int


enum PLCore_MemoryProperties {
    GPU_LOCAL = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
    CPU_VISIBLE = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
    CPU_COHERENT = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
    CPU_CACHED = VK_MEMORY_PROPERTY_HOST_CACHED_BIT,
    LAZY_ALLOCATED = VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT,
    PROTECTED_MEMORY = VK_MEMORY_PROPERTY_PROTECTED_BIT,
};


typedef struct {
    VkInstance instance;
    VkDebugUtilsMessengerEXT priv_Messenger;
} PLCore_Priv_Instance;

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
    PLCore_Priv_Instance pl_instance;
    PLCore_Device pl_device;

    PLCore_CommandPool pl_transferPool;
} PLCore_RenderInstance;




typedef struct {
    // Creation Information
    VkDescriptorPoolSize* sizes;
    uint32_t poolSizeCount;
    VkDescriptorSetLayoutBinding* bindings;
    uint32_t bindingCount;
    VkDescriptorType* types;
    uint32_t typeCount;
    uint32_t maxDescriptorSets;
    // Data Structures
    VkDescriptorPool pool;
} PLCore_DescriptorPoolAllocator;
typedef struct {
    uint32_t bindingSlot;
    uint32_t arrayCount;
    VkDescriptorType type;
    VkWriteDescriptorSet write;
    const char* name;
} PLCore_DescriptorBinding;
typedef struct {
    VkDescriptorSetLayout layout;
    VkDescriptorSet set;
    uint32_t slot;
    PLCore_DescriptorBinding* bindings;
    uint32_t bindingCount;
} PLCore_DescriptorSet;
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
    VkDeviceSize requiredSize;
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

    uint32_t backBuffers;
    uint32_t priv_activeFrame;
    uint32_t priv_imageIndex;
    VkSemaphore* priv_waitSemaphores;
    VkSemaphore* priv_signalSemaphores;
    VkFence* priv_renderFences;

} PLCore_Renderer;
typedef struct {
    FILE* file;
    char* buffer;
    size_t size;
    const char* entryPoint;
    const char* path;
    VkShaderModule module;
    VkResult result;
    VkShaderStageFlagBits stage;

    uint32_t descriptorSetCount;
    PLCore_DescriptorSet* descriptorSets;


} PLCore_ShaderModule;

//      DESCRIPTOR FUNCTIONS

PLCore_DescriptorPoolAllocator PLCore_CreateDescriptorPoolAllocator(uint32_t descriptorSlot, VkDescriptorType* types, uint32_t* descriptorSetCount, uint32_t count, uint32_t maxDescriptorSets, VkShaderStageFlagBits shaderStage);
PLCore_DescriptorPoolAllocator PLCore_CreateDescriptorPoolFromAllocator(VkDevice device, PLCore_DescriptorPoolAllocator allocator);
PLCore_DescriptorSet PLCore_CreateDescriptorSets(VkDevice device, VkDescriptorType typeFlags, PLCore_DescriptorPoolAllocator allocator);
typedef struct {
    uint32_t arrayOffset;
    uint32_t arrayCount;
} PLCore_DescriptorAdditionalInfo;
void PLCore_UpdateDescriptor(PLCore_RenderInstance instance, VkDescriptorSet set, VkDescriptorType type, uint32_t dstBinding, VkDescriptorBufferInfo* bufferInfo, VkDescriptorImageInfo* imageInfo, PLCore_DescriptorAdditionalInfo* additionalInfo);

VkDescriptorSetLayoutBinding PLCore_CreateDescriptorSetLayoutBinding(uint32_t slot, uint32_t count, VkDescriptorType type, VkShaderStageFlags stage);
VkDescriptorSet PLCore_CreateDescriptorSetAdvanced(PLCore_RenderInstance instance, VkDescriptorPool pool, uint32_t bindingCount, VkDescriptorSetLayoutBinding* bindings, VkShaderStageFlags stage, VkDescriptorSetLayout* layout);

VkDescriptorPool PLCore_CreateGeneralizedDescriptorPool(PLCore_RenderInstance instance);

VkDescriptorSet PLCore_CreateDescriptorSet(PLCore_RenderInstance instance, VkDescriptorPool pool, uint32_t slot, VkDescriptorType type, VkShaderStageFlags stage, VkDescriptorSetLayout* layout);

PLCore_DescriptorSet* PLCore_CreateDescriptorSetFromShader(PLCore_RenderInstance instance, PLCore_ShaderModule shader, VkDescriptorPool allcationPool, uint32_t* out_DescriptorCount);

VkPipelineVertexInputStateCreateInfo PLCore_CreateInputInfoFromShader(PLCore_RenderInstance instance, PLCore_ShaderModule shader);

typedef struct {
    int32_t hasShaders;
    uint32_t shaderStageCount;
    VkPipelineShaderStageCreateInfo* shaderStages;

    int32_t hasVertexInput;
    VkPipelineVertexInputStateCreateInfo vertexInput;

    int32_t hasInputAssembly;
    VkPipelineInputAssemblyStateCreateInfo inputAssembly;

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

    int32_t hasRenderPass;
    VkRenderPass renderPass;
    uint32_t subpass;
} PLCore_PipelineBuilder;
typedef struct {
    VkPipeline pipeline;
    VkPipelineLayout layout;

    PLCore_DescriptorSet* descriptorSets;

} PLCore_GraphicsPipeline;
typedef struct {
    VkBuffer buffer;
    VkDeviceMemory memory;
    VkDescriptorBufferInfo bufferInfo;
} PLCore_Buffer;
typedef struct {
    VkSampler sampler;
    VkDescriptorImageInfo samplerInfo;
} PLCore_ImageSampler;
typedef struct {
    PLCore_Image image;
    VkDescriptorImageInfo imageInfo;
} PLCore_Texture;


VkInstance              PLCore_Priv_CreateInstance(VkDebugUtilsMessengerEXT* messenger);
VkPhysicalDevice        PLCore_Priv_CreatePhysicalDevice(VkInstance instance, uint32_t* queueFamilyCount, VkQueueFamilyProperties** queueFamilies);
VkDevice                PLCore_Priv_CreateDevice(VkPhysicalDevice physicalDevice, uint32_t queueFamilyCount, VkQueueFamilyProperties* queueFamilyProperties, VkPhysicalDeviceFeatures features, uint32_t queueRequestCount, VkQueueFlagBits* queueRequest, PLCore_DeviceQueue** queues);
GLFWwindow*             PLCore_Priv_CreateWindow(VkInstance instance, uint32_t width, uint32_t height, VkSurfaceKHR* surface);
VkRenderPass            PLCore_Priv_CreateRenderPass(VkDevice device, VkFormat presentableFormat, VkFormat depthFormat);
VkCommandPool           PLCore_Priv_CreateCommandPool(VkDevice device, uint32_t queueFamily, VkCommandPoolCreateFlagBits flags);
VkCommandBuffer*        PLCore_Priv_CreateCommandBuffers(VkDevice device, VkCommandPool pool, uint32_t count);
VkSwapchainKHR          PLCore_Priv_CreateSwapchain(VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkExtent2D screenResolution, uint32_t queueFamily, VkSurfaceFormatKHR* surfaceFormat, VkPresentModeKHR* presentMode);
VkImage*                PLCore_Priv_AcquireSwapchainImages(VkDevice device, VkSwapchainKHR swapchain, VkFormat renderFormat, VkImageView** imageViews, uint32_t* imageCount);
VkImage                 PLCore_Priv_CreateDepthBuffer(VkDevice device, VkPhysicalDeviceMemoryProperties memoryProperties, VkImageView* depthView, VkFormat depthFormat, VkExtent2D resolution, uint32_t queueFamily);
VkFramebuffer           PLCore_Priv_CreateFramebuffer(VkDevice device, VkExtent2D resolution, VkRenderPass renderPass, VkImageView swapchainImage, VkImageView depthView);

PLCore_RenderInstance   PLCore_CreateRenderingInstance();
PLCore_Window           PLCore_CreateWindow(PLCore_RenderInstance instance, uint32_t width, uint32_t height);
PLCore_Renderer         PLCore_CreateRenderer(PLCore_RenderInstance instance, PLCore_Window window);
PLCore_GraphicsPipeline PLCore_CreatePipeline(PLCore_RenderInstance instance, PLCore_Renderer renderer, VkPipelineVertexInputStateCreateInfo vertexInput, PLCore_ShaderModule vertexShader, PLCore_ShaderModule fragmentShader, VkPipelineLayout* layout);

PLCore_GraphicsPipeline PLCore_CreatePipelineFromBuilder(PLCore_RenderInstance instance, PLCore_Renderer renderer, PLCore_Window window, PLCore_PipelineBuilder builder);

PLCore_Buffer           PLCore_CreateBuffer(PLCore_RenderInstance instance, VkDeviceSize size, VkBufferUsageFlagBits usage, VkMemoryPropertyFlagBits memoryFlags);
PLCore_Buffer           PLCore_CreateGPUBuffer(PLCore_RenderInstance instance, VkDeviceSize size, VkBufferUsageFlagBits usage, void* data);
PLCore_Buffer           PLCore_CreateUniformBuffer(PLCore_RenderInstance instance, VkDeviceSize size);
void                    PLCore_DestroyBuffer(PLCore_RenderInstance instance, PLCore_Buffer* buffer);
VkCommandBuffer         PLCore_ActiveRenderBuffer(PLCore_Renderer renderer);

typedef struct {
    enum beginFrameStartStage {
        BEGIN_FRAME_FROM_BEGINNING,
        CMD_BUFFER_START,
        RENDERPASS_START,
    } beginStage;
} PLCore_BeginFrameAdditionalInfo;
void PLCore_BeginFrame(PLCore_RenderInstance instance, PLCore_Renderer* renderer, PLCore_GraphicsPipeline* pipeline, PLCore_Window* window, PLCore_BeginFrameAdditionalInfo* additionalInfo);

typedef struct {
    uint32_t additionalSemaphoreCountSignal;
    VkSemaphore* additionalSemaphoresSignal;
    uint32_t additionalSemaphoreCountWait;
    VkSemaphore* additionalSemaphoresWait;
} PLCore_EndFrameAdditionalInfo;
void PLCore_EndFrame(PLCore_RenderInstance instance, PLCore_Renderer* renderer, PLCore_GraphicsPipeline* pipeline, PLCore_Window* window, PLCore_EndFrameAdditionalInfo* additionalInfo);

void PLCore_RecordCommandBuffer(VkCommandBuffer buffer);
void PLCore_StopCommandBuffer(VkCommandBuffer buffer);
void PLCore_SubmitCommandBuffer(VkCommandBuffer buffer, VkQueue queue, VkSemaphore* wait, VkSemaphore* signal, VkFence fence);

PLCore_PipelineBuilder PLCore_Priv_CreateBlankPipelineBuilder();
void PLCore_Priv_AddShadersToPipelineBuilder(PLCore_PipelineBuilder* builder, uint32_t shaderCount, VkPipelineShaderStageCreateInfo* shaders);
void PLCore_Priv_AddVertexInputToPipelineBuilder(PLCore_PipelineBuilder* builder, VkPipelineVertexInputStateCreateInfo vertexInput);
void PLCore_Priv_AddInputAssemblyToPipelineBuilder(PLCore_PipelineBuilder* builder, VkPipelineInputAssemblyStateCreateInfo inputAssembly);
void PLCore_Priv_AddPipelineLayoutToPipelineBuilder(PLCore_PipelineBuilder* builder, VkPipelineLayout pipelineLayout);
void PLCore_Priv_AddRasterizerToPipelineBuilder(PLCore_PipelineBuilder* builder, VkPipelineRasterizationStateCreateInfo rasterizer);
void PLCore_Priv_AddMultisampleStateToPipelineBuilder(PLCore_PipelineBuilder* builder, VkPipelineMultisampleStateCreateInfo multisample);
void PLCore_Priv_AddColorBlendStateToPipelineBuilder(PLCore_PipelineBuilder* builder, VkPipelineColorBlendStateCreateInfo colorBlend);
void PLCore_Priv_AddRenderPassToPipelineBuilder(PLCore_PipelineBuilder* builder, VkRenderPass renderPass);
void PLCore_Priv_AddDepthStencilToPipelineBuilder(PLCore_PipelineBuilder* builder, VkPipelineDepthStencilStateCreateInfo depthStencil);

PLCore_ShaderModule                             PLCore_CreateShader(PLCore_RenderInstance instance, const char* path, VkShaderStageFlagBits stage, const char* entryPoint, VkDescriptorPool allocationPool);
VkPipelineShaderStageCreateInfo                 PLCore_Priv_CreateShaderStage(PLCore_ShaderModule shader, VkShaderStageFlagBits shaderStage);
VkVertexInputAttributeDescription               PLCore_Priv_CreateVertexAttribute(uint32_t binding, uint32_t location, VkFormat format, uint32_t offset);
VkVertexInputBindingDescription                 PLCore_Priv_CreateVertexBinding(uint32_t binding, VkVertexInputRate inputRate, VkDeviceSize stride);
VkPipelineVertexInputStateCreateInfo            PLCore_Priv_CreateVertexInput(uint32_t attributeCount, VkVertexInputAttributeDescription* attributes, uint32_t bindingCount, VkVertexInputBindingDescription* bindings);
VkPipelineInputAssemblyStateCreateInfo          PLCore_Priv_CreateInputAssemblyStage(VkPrimitiveTopology topology);
VkViewport                                      PLCore_Priv_CreateViewport(VkExtent2D extent);
VkRect2D                                        PLCore_Priv_CreateScissor(VkExtent2D extent);
VkPipelineViewportStateCreateInfo               PLCore_Priv_CreateViewportState(VkViewport viewport, VkRect2D scissor);
VkPipelineLayout                                PLCore_Priv_CreatePipelineLayout(VkDevice device, uint32_t pushConstantCount, VkPushConstantRange* pushConstants, uint32_t setLayoutCount, VkDescriptorSetLayout* setLayouts);
VkPipelineRasterizationStateCreateInfo          PLCore_Priv_CreateRasterizer(VkPolygonMode polygonMode, VkCullModeFlagBits cullMode, VkFrontFace frontFace, float lineWidth);
VkPipelineMultisampleStateCreateInfo            PLCore_Priv_CreateMultisampleState();
VkPipelineColorBlendAttachmentState             PLCore_Priv_CreateColorBlendAttachment();
VkPipelineColorBlendStateCreateInfo             PLCore_Priv_CreateColorBlend(uint32_t attachmentCount, VkPipelineColorBlendAttachmentState attachment);
VkPipelineDepthStencilStateCreateInfo           PLCore_Priv_CreateDepthStencilState();

VkPipeline                  PLCore_Priv_CreatePipelineFromBuilder(VkDevice device, PLCore_PipelineBuilder* builder, VkPipelineLayout* layout);

VkBuffer                    PLCore_Priv_CreateBuffer(VkDevice device, VkPhysicalDeviceMemoryProperties memoryProperties, VkDeviceSize size, uint32_t queueFamily, VkBufferUsageFlagBits usage, VkMemoryPropertyFlagBits memoryFlags, VkDeviceMemory* memory);
VkBuffer                    PLCore_Priv_CreateGPUBuffer(VkDevice device, VkPhysicalDeviceMemoryProperties memoryProperties, uint32_t queueFamily, VkDeviceSize size, VkBufferUsageFlagBits usageFlags, VkCommandBuffer cmdBuffer, VkQueue submitQueue, void* data, VkDeviceMemory* memory);
void                        PLCore_UploadDataToBuffer(VkDevice device, VkDeviceMemory* memory, VkDeviceSize size, void* data);

typedef struct {
    float xyz[3];
    float rgb[3];
    float texPos[2];
    uint32_t texId;
} PLCore_Vertex;


PLCore_Image PLCore_CreateImage(VkDevice device, VkImageType type, VkFormat format, VkExtent3D extent, VkImageUsageFlagBits usage, uint32_t queueFamilyIndex, VkPhysicalDeviceMemoryProperties memoryProperties);
void PLCore_DestroyImage(VkDevice device, PLCore_Image image);
void PLCore_TransitionTextureLayout(PLCore_Buffer buffer, PLCore_Image image, uint32_t queueFamily, VkExtent3D extent, VkCommandBuffer commandBuffer, VkFence* waitFence, VkQueue submitQueue);
PLCore_Texture PLCore_CreateTexture(PLCore_RenderInstance instance, PLCore_Renderer renderer, const char* path);

PLCore_ImageSampler PLCore_CreateSampler(VkDevice device, VkFilter filter, VkSamplerAddressMode addressMode);

typedef struct {
    mat4s model, view, proj;
    float posX, posY, posZ;
    float rotX, rotY;
    float fov;
    clock_t moveTimer;
} PLCore_CameraUniform;
typedef struct {
    int buttonRight, buttonLeft, buttonForward, buttonBackward, buttonUp, buttonDown;
    float moveSpeedX, moveSpeedY, moveSpeedZ;
    int moveTime;
    int useCameraMouseMovement;
} PLCore_CameraMoveScheme;
PLCore_CameraMoveScheme PLCore_GetDefaultMoveScheme();

PLCore_CameraUniform PLCore_CreateCameraUniform();

void PLCore_PollCameraMovements(PLCore_Window window, PLCore_CameraUniform* camera, PLCore_CameraMoveScheme scheme);



void PLCore_BeginCommandBuffer(VkCommandBuffer buffer);
void PLCore_BeginRenderPass(VkCommandBuffer buffer, VkRenderPass renderPass, VkFramebuffer framebuffer, PLCore_Window* window, PLCore_Renderer* renderer);
uint32_t PLCore_GetImageIndex(PLCore_RenderInstance instance, PLCore_Renderer* renderer);
VkFramebuffer PLCore_GetActiveFrameBuffer(PLCore_Renderer* renderer);

#endif //PLUTONIUM_PLUTONIUMCORE_H
