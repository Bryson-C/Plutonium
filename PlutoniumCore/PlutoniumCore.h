//
// Created by Owner on 6/23/2022.
//

#ifndef PLUTONIUM_PLUTONIUMCORE_H
#define PLUTONIUM_PLUTONIUMCORE_H

// Plutonium core? We should open that with a screwdriver

#include <stdio.h>              // required: printf, fprintf
#include <stdlib.h>             // required: malloc, realloc, calloc
#include <string.h>             // required: memcpy
#include <assert.h>             // required: assert
#include <time.h>               // required: clock(), clock_t

#include <vulkan/vulkan.h>      // required: much functionality
#include <glfw3.h>              // required: much functionality

#include <cglm/cglm.h>          // required: matrix and vectorFunctionality
#include <cglm/struct.h>        // required: extensions of base 'cglm/cglm.h' functionality


#ifdef PLCORE_REFLECTION
    #include "SpirvReflection.h"
#endif

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

    PLCore_CommandPool pl_transferPool;
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
    #ifdef PLCORE_REFLECTION
        SpvReflectResult reflectionModuleResult;
        SpvReflectShaderModule reflectionModule;
    #endif
} PLCore_ShaderModule;
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
    PLCore_PipelineBuilder pl_builder;
} PLCore_GraphicsPipeline;
typedef struct {
    VkBuffer buffer;
    VkDeviceMemory memory;
    VkDescriptorBufferInfo bufferInfo;
} PLCore_Buffer;
typedef struct {
    PLCore_Image image;
} PLCore_Texture;
typedef struct {
    VkDescriptorPoolSize* sizes;
    VkDescriptorSetLayoutBinding* bindings;
    VkDescriptorType* types;
    uint32_t poolSizeCount;
    uint32_t totalAllocations;
} PLCore_DescriptorPoolAllocator;
typedef struct {
    VkDescriptorType type;
    uint32_t maxAllocations, currentAllocations;
    VkDescriptorPool pool;
} PLCore_DescriptorPool;
typedef struct {
    VkDescriptorSetLayout* layouts;
    VkDescriptorSet* sets;
    uint32_t count;
} PLCore_Descriptor;

#ifdef PLCORE_REFLECTION

    SpvReflectInterfaceVariable** PLCore_ShaderReflectInputVariables(PLCore_ShaderModule shaderModule, uint32_t* count);
    SpvReflectDescriptorSet** PLCore_ShaderReflectDescriptorSets(PLCore_ShaderModule shaderModule, uint32_t* count);
    SpvReflectBlockVariable** PLCore_ShaderReflectPushConstants(PLCore_ShaderModule shaderModule, uint32_t* count);

    inline void PLCore_Priv_PrintReflectionInputVariables(SpvReflectInterfaceVariable** vars, uint32_t varCount) {}
    void PLCore_Priv_PrintReflectionDescriptorSets(SpvReflectDescriptorSet** sets, uint32_t setCount);
/*
    // DEPRECATED: Shader Reflection Is Not Working
    VkPipelineLayout PLCore_CreatePipelineLayoutFromShader(PLCore_RenderInstance instance, PLCore_ShaderModule* shaderModules, uint32_t shaderCount, PLCore_Descriptor** descriptorLayouts, uint32_t* descriptorLayoutCount);
*/
#endif



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
PLCore_Window           PLCore_CreateWindow(VkInstance instance, uint32_t width, uint32_t height);
PLCore_Renderer         PLCore_CreateRenderer(PLCore_RenderInstance instance, PLCore_Window window);
PLCore_GraphicsPipeline PLCore_CreatePipeline(PLCore_RenderInstance instance, PLCore_Renderer renderer, VkPipelineVertexInputStateCreateInfo vertexInput, PLCore_ShaderModule vertexShader, PLCore_ShaderModule fragmentShader, VkPipelineLayout* layout);

PLCore_GraphicsPipeline PLCore_CreatePipelineFromBuilder(PLCore_RenderInstance instance, PLCore_Renderer renderer, PLCore_Window window, PLCore_PipelineBuilder builder);

PLCore_Buffer           PLCore_CreateBuffer(PLCore_RenderInstance instance, VkDeviceSize size, VkBufferUsageFlagBits usage, VkMemoryPropertyFlagBits memoryFlags);
PLCore_Buffer           PLCore_CreateGPUBuffer(PLCore_RenderInstance instance, VkDeviceSize size, VkBufferUsageFlagBits usage, void* data);
VkCommandBuffer         PLCore_ActiveRenderBuffer(PLCore_Renderer renderer);


void PLCore_BeginFrame(PLCore_RenderInstance instance, PLCore_Renderer* renderer, PLCore_GraphicsPipeline* pipeline, PLCore_Window* window);
void PLCore_EndFrame(PLCore_RenderInstance instance, PLCore_Renderer* renderer, PLCore_GraphicsPipeline* pipeline, PLCore_Window* window);


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

PLCore_ShaderModule                             PLCore_Priv_CreateShader(VkDevice device, const char* path, VkShaderStageFlagBits stage, const char* entryPoint);
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


typedef struct {
    PLCore_Buffer buffer;
    PLCore_Vertex* data;
    size_t dataCount;
    size_t dataSize;
    int dataChanged;
} PLCore_DynamicVertexBuffer;

PLCore_DynamicVertexBuffer  PLCore_CreateDynamicVertexBuffer();
// TODO: Vertex Becomes Invalid After Vertex Buffer Is Cleared So Maybe Cope With That
PLCore_Vertex*              PLCore_PushVerticesToDynamicVertexBuffer(PLCore_DynamicVertexBuffer* buffer, size_t elementSize, size_t elementCount, PLCore_Vertex* data);
PLCore_Buffer               PLCore_RequestDynamicVertexBufferToGPU(PLCore_RenderInstance instance, PLCore_DynamicVertexBuffer* buffer, VkBufferUsageFlagBits usage, size_t elementSize);
void                        PLCore_ClearDynamicVertexBufferData(PLCore_DynamicVertexBuffer* buffer);
void                        PLCore_MoveDynamicBufferVertices(PLCore_DynamicVertexBuffer* buffer, PLCore_Vertex* vertices, size_t vertexCount, float xOffset, float yOffset);
void                        PLCore_MoveDynamicBufferVerticesTo(PLCore_DynamicVertexBuffer* buffer, PLCore_Vertex* vertices, size_t vertexCount, float xOffset, float yOffset);


VkDescriptorSetLayoutBinding PLCore_Priv_CreateDescriptorLayoutBinding(uint32_t slot, VkDescriptorType type, uint32_t descriptorCount, VkShaderStageFlagBits stages);
VkDescriptorSetLayout PLCore_Priv_CreateDescriptorLayout(VkDevice device, uint32_t bindingCount, VkDescriptorSetLayoutBinding* bindings);
VkDescriptorPoolSize PLCore_Priv_CreateDescritorPoolSize(VkDescriptorType type, uint32_t descriptorCount);
VkDescriptorPool PLCore_Priv_CreateDescriptorPool(VkDevice device, uint32_t sets, uint32_t poolSizeCount, VkDescriptorPoolSize* sizes);
VkDescriptorSet* PLCore_Priv_CreateDescriptorSets(VkDevice device, uint32_t count, VkDescriptorType type, VkDescriptorSetLayout layout, VkDescriptorPool pool);
void PLCore_Priv_WriteDescriptor(VkDevice device, VkDescriptorSet set, VkDescriptorType type, uint32_t dstBinding, VkDescriptorBufferInfo* bufferInfo, VkDescriptorImageInfo* imageInfo);



PLCore_DescriptorPoolAllocator PLCore_CreateDescriptorPoolAllocator(uint32_t typeCount, VkDescriptorType* types, VkShaderStageFlagBits* stages, uint32_t* maxAllocations);
PLCore_DescriptorPool PLCore_CreateDescriptprPoolFromAllocator(PLCore_RenderInstance instance, PLCore_DescriptorPoolAllocator allocator);
PLCore_DescriptorPool PLCore_CreateDescriptorPool(PLCore_RenderInstance instance, VkDescriptorType type, uint32_t maxDescriptorAllocations);
PLCore_Descriptor PLCore_CreateDescriptorFromPool(PLCore_RenderInstance instance, PLCore_DescriptorPool* pool, uint32_t descriptorCount, uint32_t slot, uint32_t maxBoundAtOnce, VkShaderStageFlagBits stage);
void PLCore_UpdateDescriptor(PLCore_RenderInstance instance, VkDescriptorSet set, VkDescriptorType type, uint32_t dstBinding, VkDescriptorBufferInfo* bufferInfo, VkDescriptorImageInfo* imageInfo);



PLCore_Image PLCore_CreateImage(VkDevice device, VkImageType type, VkFormat format, VkExtent3D extent, VkImageUsageFlagBits usage, uint32_t queueFamilyIndex, VkPhysicalDeviceMemoryProperties memoryProperties);
void PLCore_DestroyImage(VkDevice device, PLCore_Image image);
void PLCore_TransitionTextureLayout(PLCore_Buffer buffer, PLCore_Image image, uint32_t queueFamily, VkExtent3D extent, VkCommandBuffer commandBuffer, VkFence* waitFence, VkQueue submitQueue);
PLCore_Texture PLCore_CreateTexture(PLCore_RenderInstance instance, PLCore_Renderer renderer, const char* path);
VkSampler PLCore_CreateSampler(VkDevice device, VkFilter filter, VkSamplerAddressMode addressMode);

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
} PLCore_CameraMoveScheme;
PLCore_CameraMoveScheme PLCore_GetDefaultMoveScheme();

PLCore_CameraUniform PLCore_CreateCameraUniform();

void PLCore_PollCameraMovements(PLCore_Window window, PLCore_CameraUniform* camera, PLCore_CameraMoveScheme scheme);



#endif //PLUTONIUM_PLUTONIUMCORE_H
