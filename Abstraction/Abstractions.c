//
// Created by Owner on 5/21/2022.
//

#include "Abstractions.h"



// Instance
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback( VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
    if (messageSeverity > VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) printf("[Vk]: %s\n\n",pCallbackData->pMessage);
    return VK_FALSE;
}
static VkDebugUtilsMessengerEXT debugMessenger;

static VkPhysicalDeviceMemoryProperties memoryProperties;
static int collectedMemoryProperties = 0;


VkInstance createInstance() {

    glfwInit();
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    VkInstance Instance;

    VkApplicationInfo appInfo;
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pNext = VK_NULL_HANDLE;
    appInfo.pEngineName = "Plutonium";
    appInfo.engineVersion = VK_MAKE_VERSION(0,1,0);
    appInfo.pApplicationName = "Application";
    appInfo.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
    appInfo.apiVersion = VK_API_VERSION_1_2;

    // TODO: Fix To Be Used Without Pre-Processor
#ifdef PL_Debug
    U32 extensionCount = 3;
    const char* extensions[] = {VK_KHR_SURFACE_EXTENSION_NAME, VK_EXT_DEBUG_UTILS_EXTENSION_NAME, "VK_KHR_win32_surface"};

    const U32 layerCount = 1;
    const char* layers[] = {"VK_LAYER_KHRONOS_validation"};
#else
    U32 extensionCount = 2;
    const char* extensions[] = {VK_KHR_SURFACE_EXTENSION_NAME, "VK_KHR_win32_surface"};

    const U32 layerCount = 0;
    const char** layers = VK_NULL_HANDLE;
#endif



    VkInstanceCreateInfo instanceInfo;
    instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
#ifdef PL_Debug
    VkDebugUtilsMessengerCreateInfoEXT debugInfo;
    debugInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    debugInfo.pNext = VK_NULL_HANDLE;
    debugInfo.flags = 0;
    debugInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    debugInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    debugInfo.pfnUserCallback = debugCallback;
    debugInfo.pUserData = VK_NULL_HANDLE;

    instanceInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugInfo;
#else
    instanceInfo.pNext = VK_NULL_HANDLE;
#endif
    instanceInfo.flags = 0;
    instanceInfo.pApplicationInfo = &appInfo;
    instanceInfo.enabledExtensionCount = extensionCount;
    instanceInfo.ppEnabledExtensionNames = extensions;
    instanceInfo.enabledLayerCount = layerCount;
    instanceInfo.ppEnabledLayerNames = layers;

    // TODO: Add Validation Layers
    vkCreateInstance(&instanceInfo, VK_NULL_HANDLE, &Instance);

#ifdef PL_Debug
    PFN_vkCreateDebugUtilsMessengerEXT debugMessengerCreation = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(Instance, "vkCreateDebugUtilsMessengerEXT");
    debugMessengerCreation(Instance, &debugInfo, VK_NULL_HANDLE, &debugMessenger);
#endif

    return Instance;
}
void destroyInstance(VkInstance* instance) {
#ifdef PL_Debug
    PFN_vkDestroyDebugUtilsMessengerEXT debugMessengerDestruction = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(*instance, "vkDestroyDebugUtilsMessengerEXT");
    debugMessengerDestruction(*instance, debugMessenger, VK_NULL_HANDLE);
#endif
    vkDestroyInstance(*instance, VK_NULL_HANDLE);
    glfwTerminate();
}

// Device
VkPhysicalDevice createPhysicalDevice(VkInstance instance) {
    VkPhysicalDevice PhysicalDevice;
    U32 deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, VK_NULL_HANDLE);
    VkPhysicalDevice* devices = malloc(sizeof(VkPhysicalDevice) * deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices);
    for (U32 i = 0; i < deviceCount; i++) {
        PhysicalDevice = devices[i];
        break;
    }
    free(devices);

    if (!collectedMemoryProperties) {
        vkGetPhysicalDeviceMemoryProperties(PhysicalDevice, &memoryProperties);
        collectedMemoryProperties = 1;
    }


    return PhysicalDevice;
}


static int isQueueInfoCollected = 0;
static U32 queueFamilyCount = 0;
static VkQueueFamilyProperties* queueFamilyProperties;
static U32* queueIndices;
static float** queuePriorities;

DeviceQueue requestDeviceQueue(VkPhysicalDevice physicalDevice, VkQueueFlagBits flags, float priority) {
    if (isQueueInfoCollected == 0) {
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, VK_NULL_HANDLE);
        queueFamilyProperties = malloc(sizeof(VkQueueFamilyProperties) * queueFamilyCount);
        queueIndices = malloc(sizeof(U32) * queueFamilyCount);
        queuePriorities = malloc(sizeof(float*) * queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilyProperties);
        for (U32 i = 0; i < queueFamilyCount; i++) {
            queueIndices[i] = 0;
            queuePriorities[i] = malloc(sizeof(float) * queueFamilyProperties[i].queueCount);
        }
        isQueueInfoCollected = 1;
    }
    DeviceQueue queue = { -1,-1 };
    for (U32 i = 0; i < queueFamilyCount; i++) {
        if ((queueFamilyProperties[i].queueFlags & flags) && queueIndices[i] < queueFamilyProperties[i].queueCount) {
            queue.familyIndex = i;
            queuePriorities[i][*queueIndices] = priority;
            queue.queueIndex = queueIndices[i];
            queueIndices[i]++;
            return queue;
        }
    }
    return queue;
}

VkDevice createDevice(VkPhysicalDevice physicalDevice) {

    VkDevice Device;

    U32 deviceExtensionCount = 1;
    const char* deviceExtensions[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    U32 deviceLayerCount = 0;
    const char** deviceLayers = VK_NULL_HANDLE;

    U32 queueInfoCount = 0;
    for (U32 i = 0; i < queueFamilyCount; i++)
        if (queueIndices[i] > 0) queueInfoCount++;

    VkDeviceQueueCreateInfo* queueInfo = malloc(sizeof(VkDeviceQueueCreateInfo) * queueInfoCount);

    for (U32 i = 0; i < queueFamilyCount; i++) {
        if (queueIndices[i] > 0) {
            queueInfo[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueInfo[i].pNext = VK_NULL_HANDLE;
            queueInfo[i].flags = 0;
            queueInfo[i].queueCount = queueIndices[i];
            queueInfo[i].queueFamilyIndex = i;
            queueInfo[i].pQueuePriorities = queuePriorities[i];
        }
    }

    VkDeviceCreateInfo deviceInfo;
    deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceInfo.pNext = VK_NULL_HANDLE;
    deviceInfo.flags = 0;
    deviceInfo.queueCreateInfoCount = queueInfoCount;
    deviceInfo.pQueueCreateInfos = queueInfo;
    deviceInfo.ppEnabledLayerNames = deviceLayers;
    deviceInfo.enabledLayerCount = deviceLayerCount;
    deviceInfo.enabledExtensionCount = deviceExtensionCount;
    deviceInfo.ppEnabledExtensionNames = deviceExtensions;
    deviceInfo.pEnabledFeatures = VK_NULL_HANDLE;

    vkCreateDevice(physicalDevice, &deviceInfo, VK_NULL_HANDLE, &Device);
    free(queueFamilyProperties);
    free(queueIndices);
    for (U32 i = 0; i < queueFamilyCount; i++)
        free(queuePriorities[i]);
    free(queuePriorities);

    return Device;
}

VkQueue getQueue(VkDevice device, DeviceQueue queue) {
    VkQueue DeviceQueue;
    vkGetDeviceQueue(device, queue.familyIndex, queue.queueIndex, &DeviceQueue);
    return DeviceQueue;
}


// Pools

VkCommandPool createCommandPool(VkDevice device, U32 queueFamily, U32 flags) {
    VkCommandPool Pool;

    VkCommandPoolCreateInfo poolInfo;
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.pNext = VK_NULL_HANDLE;
    poolInfo.flags = flags;
    poolInfo.queueFamilyIndex = queueFamily;

    vkCreateCommandPool(device, &poolInfo, VK_NULL_HANDLE, &Pool);
    return Pool;
}

VkCommandBuffer* createCommandBuffers(VkDevice device, VkCommandPool pool, U32 count) {
    VkCommandBuffer* Buffers = malloc(sizeof(VkCommandBuffer) * count);

    VkCommandBufferAllocateInfo allocInfo;
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.pNext = VK_NULL_HANDLE;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = count;
    allocInfo.commandPool = pool;

    vkAllocateCommandBuffers(device, &allocInfo, Buffers);
    return Buffers;
}

// Buffers



uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memoryProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }
}

Buffer createBuffer(VkDevice device, U32 queueFamily, VkQueue queue, VkCommandBuffer commandBuffer, VkFence fence, VkDeviceSize size, VkBufferUsageFlagBits usageFlags, void* data) {
    Buffer Buffer, StagingBuffer;



    VkBufferCreateInfo stagingInfo;
    stagingInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    stagingInfo.pNext = VK_NULL_HANDLE;
    stagingInfo.flags = 0;
    stagingInfo.size = size;
    stagingInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    stagingInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    stagingInfo.queueFamilyIndexCount = 1;
    stagingInfo.pQueueFamilyIndices = &queueFamily;
    vkCreateBuffer(device, &stagingInfo, VK_NULL_HANDLE, &StagingBuffer.buffer);

    VkMemoryRequirements sBufferRequirements;
    vkGetBufferMemoryRequirements(device, StagingBuffer.buffer, &sBufferRequirements);

    VkMemoryAllocateInfo sBufferAlloc;
    sBufferAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    sBufferAlloc.pNext = VK_NULL_HANDLE;
    sBufferAlloc.allocationSize = sBufferRequirements.size;
    sBufferAlloc.memoryTypeIndex = findMemoryType(sBufferRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    vkAllocateMemory(device, &sBufferAlloc, VK_NULL_HANDLE, &StagingBuffer.memory);
    vkBindBufferMemory(device, StagingBuffer.buffer, StagingBuffer.memory, 0);

    void* bufferData;
    vkMapMemory(device, StagingBuffer.memory, 0, stagingInfo.size, 0, &bufferData);
    memcpy(bufferData, data, (VkDeviceSize) stagingInfo.size);
    vkUnmapMemory(device, StagingBuffer.memory);

    VkBufferCreateInfo bufferInfo;
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.pNext = VK_NULL_HANDLE;
    bufferInfo.flags = 0;
    bufferInfo.size = size;
    bufferInfo.usage = usageFlags | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    bufferInfo.queueFamilyIndexCount = 1;
    bufferInfo.pQueueFamilyIndices = &queueFamily;

    vkCreateBuffer(device, &bufferInfo, VK_NULL_HANDLE, &Buffer.buffer);

    VkMemoryRequirements bufferRequirements;
    vkGetBufferMemoryRequirements(device, Buffer.buffer, &bufferRequirements);

    VkMemoryAllocateInfo BufferAlloc;
    BufferAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    BufferAlloc.pNext = VK_NULL_HANDLE;
    BufferAlloc.allocationSize = bufferRequirements.size;
    BufferAlloc.memoryTypeIndex = findMemoryType(bufferRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    vkAllocateMemory(device, &BufferAlloc, VK_NULL_HANDLE, &Buffer.memory);
    vkBindBufferMemory(device, Buffer.buffer, Buffer.memory, 0);

    VkCommandBufferBeginInfo vertexCopyBeginfo;
    vertexCopyBeginfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    vertexCopyBeginfo.pNext = VK_NULL_HANDLE;
    vertexCopyBeginfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vertexCopyBeginfo.pInheritanceInfo = VK_NULL_HANDLE;


    vkBeginCommandBuffer(commandBuffer, &vertexCopyBeginfo);

    VkBufferCopy CopyInfo;
    CopyInfo.size = stagingInfo.size;
    CopyInfo.srcOffset = 0;
    CopyInfo.dstOffset = 0;
    vkCmdCopyBuffer(commandBuffer, StagingBuffer.buffer, Buffer.buffer, 1, &CopyInfo);

    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo vertexCopySubmitInfo;
    vertexCopySubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    vertexCopySubmitInfo.pNext = VK_NULL_HANDLE;
    vertexCopySubmitInfo.waitSemaphoreCount = 0;
    vertexCopySubmitInfo.pWaitSemaphores = VK_NULL_HANDLE;
    vertexCopySubmitInfo.pWaitDstStageMask = VK_NULL_HANDLE;
    vertexCopySubmitInfo.commandBufferCount = 1;
    vertexCopySubmitInfo.pCommandBuffers = &commandBuffer;
    vertexCopySubmitInfo.signalSemaphoreCount = 0;
    vertexCopySubmitInfo.pSignalSemaphores = VK_NULL_HANDLE;

    vkQueueSubmit(queue, 1, &vertexCopySubmitInfo, fence);
    vkWaitForFences(device, 1, &fence, VK_TRUE, UINT64_MAX);
    vkResetFences(device, 1, &fence);

    destroyBuffer(device, &StagingBuffer);

    return Buffer;
}

void destroyBuffer(VkDevice device, Buffer* buffer) {
    vkDestroyBuffer(device, buffer->buffer, VK_NULL_HANDLE);
    vkFreeMemory(device, buffer->memory, VK_NULL_HANDLE);
}


// Swapchain

VkPresentModeKHR findPresentMode(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) {
    VkPresentModeKHR PresentMode;
    U32 presentModeCount = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, VK_NULL_HANDLE);
    VkPresentModeKHR* presentModes = malloc(sizeof(VkPresentModeKHR) * presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, presentModes);
    for (U32 i = 0; i < presentModeCount; i++) {
        if (presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
            PresentMode = presentModes[i];
            break;
        } else {
            PresentMode = VK_PRESENT_MODE_FIFO_KHR;
        }
    }
    free(presentModes);
    return PresentMode;
}
VkSurfaceFormatKHR findSurfaceFormat(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) {
    VkSurfaceFormatKHR SurfaceFormat;
    U32 surfaceFormatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &surfaceFormatCount, VK_NULL_HANDLE);
    VkSurfaceFormatKHR* surfaceFormats = malloc(sizeof(VkSurfaceFormatKHR) * surfaceFormatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &surfaceFormatCount, surfaceFormats);
    for (U32 i = 0; i < surfaceFormatCount; i++) {
        if (surfaceFormats[i].format == VK_FORMAT_B8G8R8A8_SRGB && surfaceFormats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            SurfaceFormat = surfaceFormats[i];
            break;
        } else {
            SurfaceFormat = surfaceFormats[0];
        }
    }
    free(surfaceFormats);
    return SurfaceFormat;
}

Swapchain createSwapchain(VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkExtent2D extent, U32 queueFamily, U32 depthBuffers) {
    Swapchain Swapchain;
    Swapchain.surfaceFormat = findSurfaceFormat(physicalDevice, surface);
    Swapchain.presentMode = findPresentMode(physicalDevice, surface);
    Swapchain.extent = extent;

    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &surfaceCapabilities);

    Swapchain.minImageCount = surfaceCapabilities.minImageCount + 1;
    if (surfaceCapabilities.maxImageCount > 0 && Swapchain.minImageCount > surfaceCapabilities.maxImageCount) {
        Swapchain.minImageCount = surfaceCapabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR swapchainInfo;
    swapchainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainInfo.pNext = VK_NULL_HANDLE;
    swapchainInfo.flags = 0;
    swapchainInfo.surface = surface;
    swapchainInfo.minImageCount = Swapchain.minImageCount;
    swapchainInfo.imageFormat = Swapchain.surfaceFormat.format;
    swapchainInfo.imageColorSpace = Swapchain.surfaceFormat.colorSpace;
    swapchainInfo.imageExtent = extent;
    swapchainInfo.imageArrayLayers = 1;
    swapchainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE; // Because Only One Queue Will Be Using The Image
    swapchainInfo.queueFamilyIndexCount = 1;
    swapchainInfo.pQueueFamilyIndices = &queueFamily;
    swapchainInfo.preTransform = surfaceCapabilities.currentTransform;
    swapchainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchainInfo.presentMode = Swapchain.presentMode;
    swapchainInfo.clipped = VK_TRUE;
    swapchainInfo.oldSwapchain = VK_NULL_HANDLE;

    vkCreateSwapchainKHR(device, &swapchainInfo, VK_NULL_HANDLE, &Swapchain.swapchain);

    Swapchain.depthFormat = VK_FORMAT_D32_SFLOAT;

    VkImageCreateInfo depthInfo;
    depthInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    depthInfo.pNext = VK_NULL_HANDLE;
    depthInfo.flags = 0;
    depthInfo.imageType = VK_IMAGE_TYPE_2D;
    depthInfo.format = Swapchain.depthFormat;
    depthInfo.extent = (VkExtent3D){.width = Swapchain.extent.width, .height = Swapchain.extent.height, 1};
    depthInfo.mipLevels = 1;
    depthInfo.arrayLayers = 1;
    depthInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    depthInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    depthInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    depthInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    depthInfo.queueFamilyIndexCount = 1;
    depthInfo.pQueueFamilyIndices = &queueFamily;
    depthInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    Swapchain.depthBuffers = depthBuffers;
    Swapchain.depthImage = malloc(sizeof(VkImage) * Swapchain.depthBuffers);
    Swapchain.depthView = malloc(sizeof(VkImageView) * Swapchain.depthBuffers);
    Swapchain.depthMemory = malloc(sizeof(VkDeviceMemory) * Swapchain.depthBuffers);

    for (U32 i = 0; i < Swapchain.depthBuffers; i++) { // per depth image

        vkCreateImage(device, &depthInfo, VK_NULL_HANDLE, &Swapchain.depthImage[i]);

        VkMemoryRequirements imageRequirements;
        vkGetImageMemoryRequirements(device, Swapchain.depthImage[i], &imageRequirements);

        VkMemoryAllocateInfo allocateInfo;
        allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocateInfo.pNext = VK_NULL_HANDLE;
        allocateInfo.allocationSize = imageRequirements.size;
        allocateInfo.memoryTypeIndex = findMemoryType(imageRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        vkAllocateMemory(device, &allocateInfo, VK_NULL_HANDLE, &Swapchain.depthMemory[i]);
        vkBindImageMemory(device, Swapchain.depthImage[i], Swapchain.depthMemory[i], 0);

        VkImageViewCreateInfo viewInfo;
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.pNext = VK_NULL_HANDLE;
        viewInfo.flags = 0;
        viewInfo.image = Swapchain.depthImage[i];
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = Swapchain.depthFormat;
        viewInfo.components.r = VK_COMPONENT_SWIZZLE_R;
        viewInfo.components.g = VK_COMPONENT_SWIZZLE_G;
        viewInfo.components.b = VK_COMPONENT_SWIZZLE_B;
        viewInfo.components.a = VK_COMPONENT_SWIZZLE_A;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.layerCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;

        vkCreateImageView(device, &viewInfo, VK_NULL_HANDLE, &Swapchain.depthView[i]);
    }


    return Swapchain;
}

FramebufferContainer createFramebufferInfo(VkDevice device, Swapchain* swapchain) {
    FramebufferContainer container;
    container.imageCount = 0;
    vkGetSwapchainImagesKHR(device, swapchain->swapchain, &container.imageCount, VK_NULL_HANDLE);
    swapchain->swapchainImageCount = container.imageCount;
    container.image = malloc(sizeof(VkImage) * container.imageCount);
    container.imageView = malloc(sizeof(VkImageView) * container.imageCount);
    container.framebuffer = malloc(sizeof(VkFramebuffer) * container.imageCount);
    vkGetSwapchainImagesKHR(device, swapchain->swapchain, &swapchain->swapchainImageCount, container.image);
    for (int i = 0; i < container.imageCount; i++) {
        VkImageViewCreateInfo viewInfo;
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.pNext = VK_NULL_HANDLE;
        viewInfo.flags = 0;
        viewInfo.image = container.image[i];
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = swapchain->surfaceFormat.format;
        viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.layerCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseMipLevel = 0;
        vkCreateImageView(device, &viewInfo, VK_NULL_HANDLE, &container.imageView[i]);
    }
    return container;
}

void createFramebuffers(VkDevice device, VkRenderPass renderPass, Swapchain swapchain, FramebufferContainer* framebufferContainer) {
    for (U32 i = 0; i < swapchain.swapchainImageCount; i++) {
        VkImageView depthView;
        if (swapchain.depthBuffers < 2) {
            depthView = swapchain.depthView[0];
        } else {
            depthView = swapchain.depthView[i];
        }

        U32 attachmentCount = 2;
        VkImageView attachments[] = {framebufferContainer->imageView[i], depthView};

        VkFramebufferCreateInfo bufferInfo;
        bufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        bufferInfo.pNext = VK_NULL_HANDLE;
        bufferInfo.flags = 0;
        bufferInfo.renderPass = renderPass;
        bufferInfo.attachmentCount = attachmentCount;
        bufferInfo.pAttachments = attachments;
        bufferInfo.width = swapchain.extent.width;
        bufferInfo.height = swapchain.extent.height;
        bufferInfo.layers = 1;
        vkCreateFramebuffer(device, &bufferInfo, VK_NULL_HANDLE, &framebufferContainer->framebuffer[i]);
    }
}

void destroySwapchain(VkDevice device, Swapchain* swapchain) {
    for (U32 i = 0; i < swapchain->depthBuffers; i++) {
        vkDestroyImage(device, swapchain->depthImage[i], VK_NULL_HANDLE);
        vkDestroyImageView(device, swapchain->depthView[i], VK_NULL_HANDLE);
        vkFreeMemory(device, swapchain->depthMemory[i], VK_NULL_HANDLE);
    }
    free(swapchain->depthImage);
    free(swapchain->depthView);
    free(swapchain->depthMemory);
    vkDestroySwapchainKHR(device, swapchain->swapchain, VK_NULL_HANDLE);
}

void destroyFramebufferContainer(VkDevice device, Swapchain swapchain, FramebufferContainer* framebufferContainers) {
    for (int i = 0; i < swapchain.swapchainImageCount; i++) {
        vkDestroyImageView(device, framebufferContainers->imageView[i], VK_NULL_HANDLE);
        vkDestroyFramebuffer(device, framebufferContainers->framebuffer[i], VK_NULL_HANDLE);
    }
}


// Pipeline

ShaderFile createShader(VkDevice device, const char* path, const char* entryPoint) {
    ShaderFile shader;
    shader.path = path;
    shader.entryPoint = entryPoint;

    fopen_s(&shader.file, path, "rb");
    fseek(shader.file, 0, SEEK_END);
    shader.size = ftell(shader.file);
    rewind(shader.file);

    shader.buffer = malloc(sizeof(char) * shader.size + 1);
    fread_s(shader.buffer, shader.size, sizeof(char), shader.size, shader.file);
    shader.buffer[shader.size-1] = '\0';

    VkShaderModuleCreateInfo shaderInfo;
    shaderInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shaderInfo.pNext = VK_NULL_HANDLE;
    shaderInfo.flags = 0;
    shaderInfo.codeSize = shader.size;
    shaderInfo.pCode = (uint32_t*)shader.buffer;

    shader.result = vkCreateShaderModule(device, &shaderInfo, VK_NULL_HANDLE, &shader.module);

    return shader;
}

VkPipelineShaderStageCreateInfo createShaderStage(ShaderFile shader, VkShaderStageFlagBits shaderStage) {
    VkPipelineShaderStageCreateInfo pipelineShader;
    pipelineShader.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    pipelineShader.flags = 0;
    pipelineShader.pNext = VK_NULL_HANDLE;
    pipelineShader.module = shader.module;
    pipelineShader.pName = shader.entryPoint;
    pipelineShader.stage = shaderStage;
    pipelineShader.pSpecializationInfo = VK_NULL_HANDLE;
    return pipelineShader;
}

VkPipelineInputAssemblyStateCreateInfo createInputAssemblyStage(VkPrimitiveTopology topology) {
    VkPipelineInputAssemblyStateCreateInfo inputAssembly;
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.pNext = VK_NULL_HANDLE;
    inputAssembly.flags = 0;
    inputAssembly.topology = topology;
    inputAssembly.primitiveRestartEnable = VK_FALSE;
    return inputAssembly;
}

VkViewport createViewport(VkExtent2D extent) {
    VkViewport viewport;
    viewport.x = 0;
    viewport.y = 0;
    viewport.width = (float)extent.width;
    viewport.height = (float)extent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    return viewport;
}

VkScissor createScissor(VkExtent2D extent) {
    VkRect2D scissor;
    scissor.extent = extent;
    scissor.offset = (VkOffset2D){.x = 0, .y = 0};
    return scissor;
}

VkPipelineViewportStateCreateInfo createViewportState(VkViewport viewport, VkScissor scissor) {
    VkPipelineViewportStateCreateInfo viewportState;
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.pNext = VK_NULL_HANDLE;
    viewportState.flags = 0;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;
    return viewportState;
}
VkPipelineLayout createPipelineLayout(VkDevice device, U32 pushConstantCount, VkPushConstantRange* pushConstants, U32 setLayoutCount, VkDescriptorSetLayout* setLayouts) {
    VkPipelineLayout pipelineLayout;

    VkPipelineLayoutCreateInfo layoutInfo;
    layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    layoutInfo.pNext = VK_NULL_HANDLE;
    layoutInfo.flags = 0;
    layoutInfo.pushConstantRangeCount = pushConstantCount;
    layoutInfo.pPushConstantRanges = pushConstants;
    layoutInfo.setLayoutCount = setLayoutCount;
    layoutInfo.pSetLayouts = setLayouts;

    vkCreatePipelineLayout(device, &layoutInfo, VK_NULL_HANDLE, &pipelineLayout);
    return pipelineLayout;
}
VkPipelineRasterizationStateCreateInfo createRasterizer(VkPolygonMode polygonMode, VkCullModeFlagBits cullMode, VkFrontFace frontFace, float lineWidth) {
    VkPipelineRasterizationStateCreateInfo rasterizer;
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.pNext = VK_NULL_HANDLE;
    rasterizer.flags = 0;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = polygonMode;
    rasterizer.cullMode = cullMode;
    rasterizer.frontFace = frontFace;
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f;
    rasterizer.depthBiasClamp = 0.0f;
    rasterizer.depthBiasSlopeFactor = 0.0f;
    rasterizer.lineWidth = lineWidth;
    return rasterizer;
}

RenderPassAttachment createRenderPassAttachment(VkImageLayout finalLayout, VkImageLayout imageType, U32 attachment, VkFormat format, VkSampleCountFlags samples,
                                                VkAttachmentLoadOp loadOp, VkAttachmentStoreOp storeOp,
                                                VkAttachmentLoadOp stencilLoadOp, VkAttachmentStoreOp stencilStoreOp) {
    RenderPassAttachment renderPassAttachment;
    renderPassAttachment.description.flags = 0;
    renderPassAttachment.description.format = format;
    renderPassAttachment.description.samples = samples;
    renderPassAttachment.description.loadOp = loadOp;
    renderPassAttachment.description.storeOp = storeOp;
    renderPassAttachment.description.stencilLoadOp = stencilLoadOp;
    renderPassAttachment.description.stencilStoreOp = stencilStoreOp;
    renderPassAttachment.description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    renderPassAttachment.description.finalLayout = finalLayout;

    renderPassAttachment.reference.layout = imageType;
    renderPassAttachment.reference.attachment = attachment;
    return renderPassAttachment;
}


void destroyShader(VkDevice device, ShaderFile* shader) {
    free(shader->buffer);
    vkDestroyShaderModule(device, shader->module, VK_NULL_HANDLE);
}

