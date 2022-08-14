//
// Created by Owner on 5/21/2022.
//

#include "Abstractions.h"



// Instance
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback( VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
    if (messageSeverity > VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) printf("[Vk]: %s\n\n",pCallbackData->pMessage);
    return VK_FALSE;
}
static VkDebugUtilsMessengerEXT s_DebugMessenger;

static VkPhysicalDeviceMemoryProperties s_PhysicalDeviceMemoryProperties;
static VkPhysicalDeviceProperties s_PhysicalDeviceProperties;
static int s_CollectedPhysicalDeviceProperties = 0;


VkInstance createInstance() {

    glfwInit();
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
    debugMessengerCreation(Instance, &debugInfo, VK_NULL_HANDLE, &s_DebugMessenger);
#endif

    return Instance;
}
void destroyInstance(VkInstance* instance) {
#ifdef PL_Debug
    PFN_vkDestroyDebugUtilsMessengerEXT debugMessengerDestruction = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(*instance, "vkDestroyDebugUtilsMessengerEXT");
    debugMessengerDestruction(*instance, s_DebugMessenger, VK_NULL_HANDLE);
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



    if (!s_CollectedPhysicalDeviceProperties) {
        vkGetPhysicalDeviceMemoryProperties(PhysicalDevice, &s_PhysicalDeviceMemoryProperties);
        vkGetPhysicalDeviceProperties(PhysicalDevice, &s_PhysicalDeviceProperties);
        printf("Max Descriptors: %u\n", s_PhysicalDeviceProperties.limits.maxBoundDescriptorSets);
        s_CollectedPhysicalDeviceProperties = 1;
    }
    InitializeBufferInfo(PhysicalDevice);

    return PhysicalDevice;
}

VkPhysicalDeviceProperties GetPhysicalDeviceProperties() { return s_PhysicalDeviceProperties; }


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

    VkPhysicalDeviceFeatures features;
    vkGetPhysicalDeviceFeatures(physicalDevice, &features);
    features.samplerAnisotropy = VK_TRUE;

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
    deviceInfo.pEnabledFeatures = &features;

    vkCreateDevice(physicalDevice, &deviceInfo, VK_NULL_HANDLE, &Device);
    free(queueFamilyProperties);
    free(queueIndices);
    for (U32 i = 0; i < queueFamilyCount; i++)
        free(queuePriorities[i]);
    free(queuePriorities);

    return Device;
}

void getQueue(VkDevice device, DeviceQueue* queue) {
    vkGetDeviceQueue(device, queue->familyIndex, queue->queueIndex, &queue->queue);
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
    for (uint32_t i = 0; i < s_PhysicalDeviceMemoryProperties.memoryTypeCount; i++)
        if ((typeFilter & (1 << i)) && (s_PhysicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
            return i;
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

VkExtent2D getSurfaceExtent(GLFWwindow* window) {
    int x,y;
    glfwGetWindowSize(window, &x, &y);
    return (VkExtent2D) {.width = x, .height = y};
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

VkVertexInputAttributeDescription createVertexAttribute(U32 binding, U32 location, VkFormat format, U32 offset) {
    VkVertexInputAttributeDescription attribute;
    attribute.binding = binding;
    attribute.location = location;
    attribute.format = format;
    attribute.offset = offset;
    return attribute;
}
VkVertexInputBindingDescription createVertexBinding(U32 binding, VkVertexInputRate inputRate, VkDeviceSize stride) {
    VkVertexInputBindingDescription description;
    description.binding = binding;
    description.inputRate = inputRate;
    description.stride = stride;
    return description;
}
VkPipelineVertexInputStateCreateInfo createVertexInput(U32 attributeCount, VkVertexInputAttributeDescription* attributes, U32 bindingCount, VkVertexInputBindingDescription* bindings) {

    VkPipelineVertexInputStateCreateInfo vertexInput;
    vertexInput.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInput.pNext = VK_NULL_HANDLE;
    vertexInput.flags = 0;
    vertexInput.vertexBindingDescriptionCount= bindingCount;
    vertexInput.pVertexBindingDescriptions = bindings;
    vertexInput.vertexAttributeDescriptionCount = attributeCount;
    vertexInput.pVertexAttributeDescriptions = attributes;
    return vertexInput;
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
    // It is important to send local (to the main function) variables so that the memory is not un-initialized when the struct is returned
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



VkPipelineMultisampleStateCreateInfo createMultisampleState() {
    VkPipelineMultisampleStateCreateInfo multisampling;
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.pNext = VK_NULL_HANDLE;
    multisampling.flags = 0;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.0f; // Optional
    multisampling.pSampleMask = VK_NULL_HANDLE; // Optional
    multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
    multisampling.alphaToOneEnable = VK_FALSE; // Optional
    return multisampling;
}
VkPipelineColorBlendAttachmentState createColorBlendAttachment() {
    VkPipelineColorBlendAttachmentState colorBlendAttachment;
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional
    return colorBlendAttachment;
}
VkPipelineColorBlendStateCreateInfo createColorBlend(U32 attachmentCount, VkPipelineColorBlendAttachmentState attachment) {
    VkPipelineColorBlendStateCreateInfo colorBlend;
    colorBlend.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlend.pNext = VK_NULL_HANDLE;
    colorBlend.flags = 0;
    colorBlend.logicOpEnable = VK_TRUE; // TODO: does this enable transparency?
    colorBlend.logicOp = VK_LOGIC_OP_COPY;
    colorBlend.attachmentCount = attachmentCount;
    colorBlend.pAttachments = &attachment;
    colorBlend.blendConstants[0] = 0.0f;
    colorBlend.blendConstants[1] = 0.0f;
    colorBlend.blendConstants[2] = 0.0f;
    colorBlend.blendConstants[3] = 0.0f;
    return colorBlend;
}
VkPipelineDepthStencilStateCreateInfo createDepthStencilState() {
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
    return depthStencil;
}


// render pass
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

RenderPass createRenderPass(VkDevice device, Swapchain swapchain) {
    RenderPass renderPass;

    renderPass.attachmentCount = 2;

    RenderPassAttachment colorAttachment = createRenderPassAttachment(VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                                                                      VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                                                      0,
                                                                      swapchain.surfaceFormat.format,
                                                                      VK_SAMPLE_COUNT_1_BIT,
                                                                      VK_ATTACHMENT_LOAD_OP_CLEAR,
                                                                      VK_ATTACHMENT_STORE_OP_STORE,
                                                                      VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                                                                      VK_ATTACHMENT_STORE_OP_DONT_CARE);
    RenderPassAttachment depthAttachment = createRenderPassAttachment(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                                                                      VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                                                                      1,
                                                                      swapchain.depthFormat,
                                                                      VK_SAMPLE_COUNT_1_BIT,
                                                                      VK_ATTACHMENT_LOAD_OP_CLEAR,
                                                                      VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                                                      VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                                                                      VK_ATTACHMENT_STORE_OP_DONT_CARE);


    renderPass.descriptions = malloc(sizeof(VkAttachmentDescription) * renderPass.attachmentCount);
    renderPass.descriptions[0] = colorAttachment.description;
    renderPass.descriptions[1] = depthAttachment.description;
    renderPass.references = malloc(sizeof(VkAttachmentReference) * renderPass.attachmentCount);
    renderPass.references[0] = colorAttachment.reference;
    renderPass.references[1] = depthAttachment.reference;


    renderPass.subpassCount = 1;
    renderPass.subpass.flags = 0;
    renderPass.subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    renderPass.subpass.inputAttachmentCount = 0;
    renderPass.subpass.pInputAttachments = VK_NULL_HANDLE;
    renderPass.subpass.colorAttachmentCount = 1;
    renderPass.subpass.pColorAttachments = &renderPass.references[0];
    renderPass.subpass.pResolveAttachments = VK_NULL_HANDLE;
    renderPass.subpass.pDepthStencilAttachment = &renderPass.references[1];
    renderPass.subpass.preserveAttachmentCount = 0;
    renderPass.subpass.pPreserveAttachments = VK_NULL_HANDLE;

    renderPass.dependencyCount = 1;
    renderPass.dependency.dependencyFlags = 0;
    renderPass.dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    renderPass.dependency.dstSubpass = 0;
    renderPass.dependency.srcAccessMask = 0;
    renderPass.dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    renderPass.dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    renderPass.dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo renderPassInfo;
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.pNext = VK_NULL_HANDLE;
    renderPassInfo.flags = 0;
    renderPassInfo.attachmentCount = renderPass.attachmentCount;
    renderPassInfo.pAttachments = renderPass.descriptions;
    renderPassInfo.subpassCount = renderPass.subpassCount;
    renderPassInfo.pSubpasses = &renderPass.subpass;
    renderPassInfo.dependencyCount = renderPass.dependencyCount;
    renderPassInfo.pDependencies = &renderPass.dependency;

    vkCreateRenderPass(device, &renderPassInfo, VK_NULL_HANDLE, &renderPass.renderPass);


    renderPass.clearCount = 2;
    renderPass.clears = malloc(sizeof(VkClearValue) * renderPass.clearCount);
    renderPass.clears[0].color.float32[0] = 0.015f;
    renderPass.clears[0].color.float32[1] = 0.015f;
    renderPass.clears[0].color.float32[2] = 0.015f;
    renderPass.clears[0].color.float32[3] = 1.0f;

    renderPass.clears[1].depthStencil.depth = 1.0f;
    renderPass.clears[1].depthStencil.stencil = 0;


    return renderPass;
}


PipelineBuilder newPipelineBuilder() {
    PipelineBuilder builder;
    builder.hasShaders = 0;
    builder.hasVertexInput = 0;
    builder.hasDepthStencil = 0;
    builder.hasInputAssembly = 0;
    builder.hasPipelineLayout = 0;
    builder.hasMultisample = 0;
    builder.hasColorBlend = 0;
    builder.hasRasterizer = 0;
    builder.hasRenderPass = 0;
    builder.hasViewportState = 0;
    builder.hasScissor = 0;
    builder.hasViewport = 0;
    builder.hasExtent = 0;
    return builder;
}

void addShadersToPipelineBuilder(PipelineBuilder* builder, U32 shaderCount, VkPipelineShaderStageCreateInfo* shaders) {
    builder->hasShaders = 1;
    builder->shaderStages = shaders;
    builder->shaderStageCount = shaderCount;
}

void addVertexInputToPipelineBuilder(PipelineBuilder* builder, VkPipelineVertexInputStateCreateInfo vertexInput) {
    builder->hasVertexInput = 1;
    builder->vertexInput = vertexInput;
}

void addInputAssemblyToPipelineBuilder(PipelineBuilder* builder, VkPipelineInputAssemblyStateCreateInfo inputAssembly) {
    builder->hasInputAssembly = 1;
    builder->inputAssembly = inputAssembly;
}

void addExtent2dToPipelineBuilder(PipelineBuilder* builder, VkExtent2D extent) {
    builder->hasExtent = 1;
    builder->extent = extent;
}

void addViewportStateToPipelineBuilder(PipelineBuilder* builder, VkPipelineViewportStateCreateInfo viewportState) {
    builder->hasViewportState = 1;
    builder->viewportState = viewportState;
}

void addPipelineLayoutToPipelineBuilder(PipelineBuilder* builder, VkPipelineLayout pipelineLayout) {
    builder->hasPipelineLayout = 1;
    builder->pipelineLayout = pipelineLayout;
}

void addRasterizerToPipelineBuilder(PipelineBuilder* builder, VkPipelineRasterizationStateCreateInfo rasterizer) {
    builder->hasRasterizer = 1;
    builder->rasterizer = rasterizer;
}

void addMultisampleStateToPipelineBuilder(PipelineBuilder* builder, VkPipelineMultisampleStateCreateInfo multisample) {
    builder->hasMultisample = 1;
    builder->multisample = multisample;
}

void addColorBlendStateToPipelineBuilder(PipelineBuilder* builder, VkPipelineColorBlendStateCreateInfo colorBlend) {
    builder->hasColorBlend = 1;
    builder->colorBlend = colorBlend;
}

void addRenderPassToPipelineBuilder(PipelineBuilder* builder, RenderPass renderPass) {
    builder->hasRenderPass = 1;
    builder->renderPass = renderPass;
}

void addDepthStencilToPipelineBuilder(PipelineBuilder* builder, VkPipelineDepthStencilStateCreateInfo depthStencil) {
    builder->hasDepthStencil = 1;
    builder->depthStencil = depthStencil;
}

VkPipeline createPipelineFromBuilder(VkDevice device, Swapchain swapchain, PipelineBuilder* builder) {
    VkPipeline pipeline;

    // Cannot Go Without Shaders
    if (builder->hasShaders == 0) {}
    if (builder->hasVertexInput == 0) {}

    if (builder->hasDepthStencil == 0) { addDepthStencilToPipelineBuilder(builder, createDepthStencilState()); }
    if (builder->hasInputAssembly == 0) { addInputAssemblyToPipelineBuilder(builder, createInputAssemblyStage(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)); }
    if (builder->hasPipelineLayout == 0) { addPipelineLayoutToPipelineBuilder(builder, createPipelineLayout(device, 0, VK_NULL_HANDLE, 0, VK_NULL_HANDLE)); }
    if (builder->hasMultisample == 0) { addMultisampleStateToPipelineBuilder(builder, createMultisampleState()); }
    if (builder->hasColorBlend == 0) { addColorBlendStateToPipelineBuilder(builder, createColorBlend(1, createColorBlendAttachment())); }
    if (builder->hasRasterizer == 0) { addRasterizerToPipelineBuilder(builder, createRasterizer(VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_CLOCKWISE, 1.0f)); }

    // Cannot Go Without Viewport or Extent
    if (builder->hasViewportState == 0) {
        if (builder->hasExtent != 0) { // if we have extent
            if (builder->hasScissor == 0) builder->viewport = createViewport(builder->extent);
            if (builder->hasViewport == 0) builder->scissor = createScissor(builder->extent);
        } else { /* cannot recover*/ }
        builder->viewportState = createViewportState(builder->viewport, builder->scissor);
    }

    if (builder->hasRenderPass == 0) { builder->renderPass = createRenderPass(device, swapchain); addRenderPassToPipelineBuilder(builder, builder->renderPass); }

    VkGraphicsPipelineCreateInfo pipelineInfo;
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.pNext = VK_NULL_HANDLE;
    pipelineInfo.flags = 0;
    pipelineInfo.stageCount = builder->shaderStageCount;
    pipelineInfo.pStages = builder->shaderStages;
    pipelineInfo.pVertexInputState = &builder->vertexInput;
    pipelineInfo.pInputAssemblyState = &builder->inputAssembly;
    pipelineInfo.pTessellationState = VK_NULL_HANDLE;
    pipelineInfo.pViewportState = &builder->viewportState;
    pipelineInfo.pRasterizationState = &builder->rasterizer;
    pipelineInfo.pMultisampleState = &builder->multisample;
    pipelineInfo.pDepthStencilState = &builder->depthStencil;
    pipelineInfo.pColorBlendState = &builder->colorBlend;
    pipelineInfo.pDynamicState = VK_NULL_HANDLE;
    pipelineInfo.layout = builder->pipelineLayout;
    pipelineInfo.renderPass = builder->renderPass.renderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.basePipelineIndex = -1;

    vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, VK_NULL_HANDLE, &pipeline);
    return pipeline;
}


void destroyShader(VkDevice device, ShaderFile* shader) {
    free(shader->buffer);
    vkDestroyShaderModule(device, shader->module, VK_NULL_HANDLE);
}


// Drawing

void beginFrameRecording(VkCommandBuffer* buffer, RenderPass renderPass, VkFramebuffer framebuffer, VkScissor scissor) {
    VkRenderPassBeginInfo beginfo;
    beginfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    beginfo.pNext = VK_NULL_HANDLE;
    beginfo.renderPass = renderPass.renderPass;
    beginfo.framebuffer = framebuffer;
    beginfo.renderArea = scissor;
    beginfo.clearValueCount = renderPass.clearCount;
    beginfo.pClearValues = renderPass.clears;

    VkCommandBufferBeginInfo cmdBeginfo;
    cmdBeginfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    cmdBeginfo.pNext = VK_NULL_HANDLE;
    cmdBeginfo.flags = 0;
    cmdBeginfo.pInheritanceInfo = VK_NULL_HANDLE;

    vkBeginCommandBuffer(*buffer, &cmdBeginfo);
    vkCmdBeginRenderPass(*buffer, &beginfo, VK_SUBPASS_CONTENTS_INLINE);
}

void endFrameRecording(VkCommandBuffer* buffer) {
    vkCmdEndRenderPass(*buffer);
    vkEndCommandBuffer(*buffer);
}


VkFence* createFences(VkDevice device, U32 count) {
    VkFence* fences = malloc(sizeof(VkFence) * count);
    for (U32 i = 0; i < count; i++) {
        VkFenceCreateInfo fenceInfo;
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        fenceInfo.pNext = VK_NULL_HANDLE;
        vkCreateFence(device, &fenceInfo, VK_NULL_HANDLE, &fences[i]);
    }
    return fences;
}

VkSemaphore* createSemaphores(VkDevice device, U32 count) {
    VkSemaphore* sema = malloc(sizeof(VkSemaphore) * count);
    for (U32 i = 0; i < count; i++) {
        VkSemaphoreCreateInfo semaInfo;
        semaInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        semaInfo.flags = 0;
        semaInfo.pNext = VK_NULL_HANDLE;
        vkCreateSemaphore(device, &semaInfo, VK_NULL_HANDLE, &sema[i]);
    }
    return sema;
}

void destroyFences(VkDevice device, U32 fenceCount, VkFence* fences) {
    for (U32 i = 0; i < fenceCount; i++) vkDestroyFence(device, fences[i], VK_NULL_HANDLE);
}

void destroySemaphores(VkDevice device, U32 semaphoreCount, VkSemaphore* semaphores) {
    for (U32 i = 0; i < semaphoreCount; i++) vkDestroySemaphore(device, semaphores[i], VK_NULL_HANDLE);
}



Image createImage(VkDevice device, VkImageType type, VkFormat format, VkExtent3D extent, VkImageUsageFlagBits usage, DeviceQueue queue) {
    Image image;

    VkImageCreateInfo imageInfo;
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.pNext = VK_NULL_HANDLE;
    imageInfo.flags = 0;
    imageInfo.imageType = type;
    imageInfo.format = format;
    imageInfo.extent = extent;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.usage = usage;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.queueFamilyIndexCount = 1;
    imageInfo.pQueueFamilyIndices = &queue.familyIndex;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    vkCreateImage(device, &imageInfo, VK_NULL_HANDLE, &image.image);

    VkMemoryRequirements requirements;
    vkGetImageMemoryRequirements(device, image.image, &requirements);

    VkMemoryAllocateInfo allocInfo;
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.pNext = VK_NULL_HANDLE;
    allocInfo.allocationSize = requirements.size;
    // this means we will be allocating the image to the gpu
    allocInfo.memoryTypeIndex = findMemoryType(requirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    vkAllocateMemory(device, &allocInfo, VK_NULL_HANDLE, &image.memory);
    vkBindImageMemory(device, image.image, image.memory, 0);

    VkImageViewCreateInfo viewInfo;
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.pNext = VK_NULL_HANDLE;
    viewInfo.flags = 0;
    viewInfo.image = image.image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.layerCount = 1;
    viewInfo.subresourceRange.levelCount = 1;

    vkCreateImageView(device, &viewInfo, VK_NULL_HANDLE, &image.view);

    image.requiredSize = requirements.size;
    return image;
}
void destroyImage(VkDevice device, Image image) {
    vkDestroyImageView(device, image.view, VK_NULL_HANDLE);
    vkDestroyImage(device, image.image, VK_NULL_HANDLE);
    vkFreeMemory(device, image.memory, VK_NULL_HANDLE);
}

void TransitionTextureLayout(VkDevice device, Buffer buffer, Image image, U32 queueFamily, VkExtent3D extent) {
    VkCommandBuffer cmd = AcquireGlobalCommandBuffer(device);

    VkCommandBufferBeginInfo beginInfo;
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.pNext = VK_NULL_HANDLE;
    beginInfo.flags = 0;
    beginInfo.pInheritanceInfo = VK_NULL_HANDLE;

    vkBeginCommandBuffer(cmd, &beginInfo);

    VkImageMemoryBarrier barrier;
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.pNext = VK_NULL_HANDLE;
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.srcQueueFamilyIndex = queueFamily;
    barrier.dstQueueFamilyIndex = queueFamily;
    barrier.image = image.image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.subresourceRange.levelCount = 1;

    vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, VK_NULL_HANDLE, 0, VK_NULL_HANDLE, 1, &barrier);

    VkBufferImageCopy copy;
    copy.bufferOffset = 0;
    copy.bufferRowLength = 0;
    copy.bufferImageHeight = 0;
    copy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copy.imageSubresource.layerCount = 1;
    copy.imageSubresource.baseArrayLayer = 0;
    copy.imageSubresource.mipLevel = 0;
    copy.imageOffset = (VkOffset3D){0,0,0};
    copy.imageExtent = extent;

    vkCmdCopyBufferToImage(cmd, buffer.buffer, image.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy);

    VkImageMemoryBarrier finalBarrier = barrier;
    finalBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    finalBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    finalBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    finalBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, VK_NULL_HANDLE, 0, VK_NULL_HANDLE, 1, &finalBarrier);
    vkEndCommandBuffer(cmd);
    GlobalSubmit(cmd);
}
// TODO: Make Texture Consume Descriptor Set (Instead Of Creating A New Descriptor)
Texture CreateTexture(VkDevice device, DeviceQueue queue, VkDescriptorSet set, U32 setBinding, const char* path) {
    int32_t width, height, channels;
    void* pixels = stbi_load(path, &width, &height, &channels, STBI_rgb_alpha);
    if (pixels == VK_NULL_HANDLE) fprintf(stderr, "%s", "big sad: loading images\n");

    VkDeviceSize imageSize = width * height * 4;
    VkFormat imageFormat = VK_FORMAT_R8G8B8A8_SRGB;
    VkExtent3D imageExtent = (VkExtent3D){width, height, 1};

    Image textureImage = createImage(device, VK_IMAGE_TYPE_2D, imageFormat, imageExtent, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, queue);

    Buffer stagingBuffer = CreateBuffer(device, imageSize, queue.familyIndex, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, CPU_COHERENT | CPU_VISIBLE);

    UploadDataToBuffer(device, &stagingBuffer, imageSize, pixels);

    stbi_image_free(pixels);





    TransitionTextureLayout(device, stagingBuffer, textureImage, queue.familyIndex, imageExtent);

    Texture texture;
    texture.image = textureImage;
    texture.set = set;
    texture.index = -1; //Because We Dont Know/Need An Index Currently

    destroyBuffer(device, &stagingBuffer);

    return texture;
}



VkSampler createSampler(VkDevice device, VkFilter filter, VkSamplerAddressMode addressMode) {
    VkSampler sampler;

    VkSamplerCreateInfo samplerInfo;
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.pNext = VK_NULL_HANDLE;
    samplerInfo.flags = 0;
    samplerInfo.magFilter = filter;
    samplerInfo.minFilter = filter;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.addressModeU = addressMode;
    samplerInfo.addressModeV = addressMode;
    samplerInfo.addressModeW = addressMode;
    samplerInfo.mipLodBias = 0.0f;

    samplerInfo.anisotropyEnable = VK_TRUE;
    samplerInfo.maxAnisotropy = 1.0f;

    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f;
    samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;

    vkCreateSampler(device, &samplerInfo, VK_NULL_HANDLE, &sampler);
    return sampler;
}

void destroySampler(VkDevice device, VkSampler sampler) {
    vkDestroySampler(device, sampler, VK_NULL_HANDLE);
}





