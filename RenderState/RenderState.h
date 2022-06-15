//
// Created by Owner on 6/5/2022.
//

#ifndef PLUTONIUM_RENDERSTATE_H
#define PLUTONIUM_RENDERSTATE_H


#include <stdbool.h>
#include <time.h>
#include <math.h>


#include "vulkan/vulkan.h"

#include "../Abstraction/Abstractions.h"


#define API_VULKAN 1


#ifdef API_VULKAN
typedef VkInstance Instance;
typedef VkDevice LogicalDevice;
typedef VkPhysicalDevice PhysicalDevice;
typedef VkCommandPool CommandPool;
typedef VkCommandBuffer CommandBuffer;
typedef VkExtent2D Extent2D;
typedef VkPipelineLayout PipelineLayout;
typedef VkPipeline Pipeline;
#else
#endif





typedef struct {
    float x,y,z;
} Float3;

typedef struct {
    float x,y;
} Float2;

inline Float3 newFloat3(const float x, const float y, const float z) {
    return (Float3){.x = x, .y = y, .z = z};
}
inline Float2 newFloat2(const float x, const float y) {
    return (Float2) {.x = x, .y = y};
}


typedef struct {
    Float3 pos;
    Float3 col;
    Float2 uv;
} vertex;

typedef struct {
    Float3 xyz;
} UBO;


inline void printQueueFlags(VkQueueFamilyProperties family) {
    if (family.queueFlags & VK_QUEUE_GRAPHICS_BIT) { printf(" Graphics "); }
    if (family.queueFlags & VK_QUEUE_SPARSE_BINDING_BIT) { printf(" Sparse Binding "); }
    if (family.queueFlags & VK_QUEUE_TRANSFER_BIT) { printf(" Transfer "); }
    if (family.queueFlags & VK_QUEUE_COMPUTE_BIT) { printf(" Compute "); }
}


enum shaderType {
    shaderFloat = VK_FORMAT_R32_SFLOAT,
    shaderVec2 = VK_FORMAT_R32G32_SFLOAT,
    shaderVec3 = VK_FORMAT_R32G32B32_SFLOAT,
    shaderVec4 = VK_FORMAT_R32G32B32A32_SFLOAT,
};




#define MAX_TRANSFER_CMD_BUFFERS 5
#define MAX_BUFFER_REQUESTS 20
#define MAX_SHADERS 20

// Begin RenderState

struct VulkanRenderState_t {
    U32 backBuffers, activeFrame, imageIndex;

    Instance instance;
    PhysicalDevice physicalDevice;
    LogicalDevice device;
    DeviceQueue graphicsQueue, transferQueue;
    CommandPool graphicsPool, transferPool;
    CommandBuffer* renderBuffers, *transferBuffers;
    VkFence* renderFences, *transferFences;
    VkSemaphore* waitSemaphore, *signalSemaphore;
    GLFWwindow* window;
    VkSurfaceKHR surface;


    Swapchain swapchain;
    FramebufferContainer framebuffers;
    Extent2D extent;
    RenderPass renderPass;
    PipelineLayout pipelineLayout;
    Pipeline graphicsPipeline;
};
typedef struct VulkanRenderState_t VulkanRenderState;

VulkanRenderState VRS;



void CreateRenderState();
void CreateRenderStateRenderer();

void DestroyRenderState();
void DestroyRenderStateRenderer();

void BeginDraw();
void SubmitDraw();



// End RenderState

// Begin RenderState Communications

struct VRSShaderInfo_t { const char* path, *entryPoint; VkShaderStageFlagBits stage; };
typedef struct VRSShaderInfo_t VRSShaderInfo;

void ApplyShader(VkShaderStageFlagBits stage, const char* path, const char* entryPoint);

void ApplyVertexInput(U32 attributeCount, VkVertexInputAttributeDescription* attributes, U32 bindingCount, VkVertexInputBindingDescription* bindings);

void ApplyPushConstants(U32 count, VkPushConstantRange* ranges);

void ApplyDescriptorLayouts(U32 count, VkDescriptorSetLayout* layouts);


Buffer RequestBuffer(VkDeviceSize size, VkBufferUsageFlagBits usage, void* data, bool useStaging);




// End RenderState Communications



#endif //PLUTONIUM_RENDERSTATE_H
