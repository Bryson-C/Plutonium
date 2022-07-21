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

    VkDevice device;
} PLCore_Device;
typedef struct {
    PLCore_Instance instance;
    PLCore_Device device;
} PLCore_RenderInstance;

VkInstance          PLCore_Priv_CreateInstance(VkDebugUtilsMessengerEXT* messenger);
VkPhysicalDevice    PLCore_Priv_CreatePhysicalDevice(VkInstance instance, uint32_t* queueFamilyCount, VkQueueFamilyProperties** queueFamilies);
VkDevice            PLCore_Priv_CreateDevice(VkPhysicalDevice physicalDevice, uint32_t queueFamilyCount, VkQueueFamilyProperties* queueFamilyProperties, VkPhysicalDeviceFeatures features, uint32_t queueRequestCount, VkQueueFlagBits* queueRequest, PLCore_DeviceQueue** queues);
VkRenderPass        PLCore_Priv_CreateRenderPass();
VkSwapchainKHR      PLCore_Priv_CreateSwapchain();

PLCore_RenderInstance   PLCore_CreateRenderingInstance();



#endif //PLUTONIUM_PLUTONIUMCORE_H
