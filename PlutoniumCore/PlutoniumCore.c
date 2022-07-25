//
// Created by Owner on 6/23/2022.
//

#include "PlutoniumCore.h"


static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback( VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
    if (messageSeverity > VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) printf("[Vk]: %s\n\n",pCallbackData->pMessage);
    return VK_FALSE;
}
static uint32_t findMemoryType(VkPhysicalDeviceMemoryProperties memoryProperties, uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++)
        if ((typeFilter & (1 << i)) && (memoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
            return i;
}




VkInstance PLCore_Priv_CreateInstance(VkDebugUtilsMessengerEXT* messenger) {

    // Initialize GLFW Library
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    VkInstance Instance;

    VkApplicationInfo appInfo = {
            .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
            .pNext = VK_NULL_HANDLE,
            .pEngineName = "Plutonium",
            .engineVersion = VK_MAKE_VERSION(0, 1, 0),
            .pApplicationName = "Application",
            .applicationVersion = VK_MAKE_VERSION(0, 1, 0),
            .apiVersion = VK_API_VERSION_1_2,
    };

    // Create Layer And Extensions String Arrays To Be Used For Instance Creation
    uint32_t extensionCount = 2;
    const char** extensions = malloc(sizeof(char*) * extensionCount);
    extensions[0] = malloc(sizeof(char) * 256);
    extensions[0] = VK_KHR_SURFACE_EXTENSION_NAME;
    extensions[1] = malloc(sizeof(char) * 256);
    extensions[1] = "VK_KHR_win32_surface";

    uint32_t layerCount = 0;
    const char** layers = VK_NULL_HANDLE;

    // If Debug > 0; Add The Validation Extension And Layer To The Currently Used Extension And Layer Variables
    int debug = 1;
    if (debug > 0) {
        extensions = realloc(extensions, sizeof(char*) * (++extensionCount));
        extensions[extensionCount - 1] = malloc(sizeof(char) * 256);
        extensions[extensionCount - 1] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;

        layers = malloc(sizeof(char*) * (++layerCount));
        layers[layerCount - 1] = malloc(sizeof(char) * 256);
        layers[layerCount - 1] = "VK_LAYER_KHRONOS_validation";
    }

    // How The Validation Messenger Should Be Created
    // We Create This But It Is Not Always Going To Be Used
    VkDebugUtilsMessengerCreateInfoEXT debugInfo = {
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
            .pNext = VK_NULL_HANDLE,
            .flags = 0,
            .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                               VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                               VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
            .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
            .pfnUserCallback = debugCallback,
            .pUserData = VK_NULL_HANDLE,
    };

    // The Validation Messenger To Be Passed Into The Instance Create Info
    const void* validationMessenger;
    if (debug > 0)
        validationMessenger = (VkDebugUtilsMessengerCreateInfoEXT *) &debugInfo;
    else
        validationMessenger = VK_NULL_HANDLE;

    VkInstanceCreateInfo instanceInfo = {
            .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            .flags = 0,
            .pNext = validationMessenger,
            .pApplicationInfo = &appInfo,
            .enabledExtensionCount = extensionCount,
            .ppEnabledExtensionNames = extensions,
            .enabledLayerCount = layerCount,
            .ppEnabledLayerNames = layers,
    };
    vkCreateInstance(&instanceInfo, VK_NULL_HANDLE, &Instance);

    // Ask Program To Obtain `vkCreateDebugUtilsMessengerEXT` Passed Into Instance Object; And Wrap It In A "Lambda" Type Function
    PFN_vkCreateDebugUtilsMessengerEXT debugMessengerCreation = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(Instance, "vkCreateDebugUtilsMessengerEXT");
    // Call The Recently Created "Lambda" To Create The Validation Messenger
    debugMessengerCreation(Instance, &debugInfo, VK_NULL_HANDLE, messenger);

    // Free Heap Allocated String Array
    // TODO: Free Individual Strings Inside Of Extensions
    free(extensions);

    // Free Heap Allocated String Array
    // TODO: Free Individual Strings Inside Of Layers
    free(layers);


    return Instance;
}
VkPhysicalDevice PLCore_Priv_CreatePhysicalDevice(VkInstance instance, uint32_t* queueFamilyCount, VkQueueFamilyProperties** queueFamilies) {
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    uint32_t deviceCount = 0;

    // Search Through All Available Graphics Cards
    vkEnumeratePhysicalDevices(instance, &deviceCount, VK_NULL_HANDLE);
    VkPhysicalDevice* devices = malloc(sizeof(VkPhysicalDevice) * deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices);

    // Give Error If No Graphics Card Is Available
    if (deviceCount < 1) {
        fprintf(stderr, "No Graphics Cards Were Found!\n");
        assert(0);
    }

    // Search Through All Graphics Cards Found
    // TODO: Select Graphics Card Which Is The Best For The Application
    for (uint32_t i = 0; i < deviceCount; i++) { physicalDevice = devices[i]; break; }

    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, queueFamilyCount, VK_NULL_HANDLE);
    *queueFamilies = malloc(sizeof(VkQueueFamilyProperties) * (*queueFamilyCount));
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, queueFamilyCount, *queueFamilies);

    // Free List Cause The Selected Device Has Been Copied To The `physicalDevice` Variable
    free(devices);

    return physicalDevice;
}
VkDevice PLCore_Priv_CreateDevice(VkPhysicalDevice physicalDevice, uint32_t queueFamilyCount, VkQueueFamilyProperties* queueFamilyProperties, VkPhysicalDeviceFeatures features, uint32_t queueRequestCount, VkQueueFlagBits* queueRequest, PLCore_DeviceQueue** queues) {

    // These Structures Are Created For Later Use Inside Of QueueInfos Structure (Which Then Is Used For Creating The Device)
    float** queuePriorities = malloc(sizeof(float*) * queueFamilyCount);
    uint32_t* queueIndices = calloc(queueFamilyCount, sizeof(uint32_t));
    for (uint32_t i = 0; i < queueFamilyCount; i++) {
        queuePriorities[i] = calloc(queueFamilyProperties[i].queueCount, sizeof(float));
    }

    // The Parameter Needs To Be Initialized However We Cannot Initialize It, Ergo It Must Be Null When Passed As A Parameter
    // When Passed As Null It Will Then Be Freshly Allocated And Returned
    *queues = malloc(sizeof(PLCore_DeviceQueue) * queueRequestCount);

    // Goes Through All Families Or Until All Queues Have Data To Be Created With
    for (int family = 0, flag = 0; family < queueFamilyCount && flag < 2; family++) {
        // If The QueueRequest Flag Is Found We Can Then Initialize The Queue
        if (queueFamilyProperties[family].queueFlags & queueRequest[flag] && queueIndices[family] < queueFamilyProperties[family].queueCount) {
            queuePriorities[family][*queueIndices] = 1.0f;
            (*queues)[flag].flag = queueFamilyProperties[family].queueFlags;
            (*queues)[flag].familyIndex = family;
            (*queues)[flag].queueIndex = queueIndices[family]++;
            flag++;
        }
    }

    // Loops Through All QueueIndices (Previously Recorded In Loop Above)
    // If The Current Array Element Has 0 We Can Skip As There Is No Queues To Be Created From That Family
    uint32_t queueInfoCount = 0;
    for (uint32_t i = 0; i < queueFamilyCount; i++)
        if (queueIndices[i] > 0) queueInfoCount++;

    // Create `x` Queue Infos Based On Loop Above
    VkDeviceQueueCreateInfo* queueInfo = malloc(sizeof(VkDeviceQueueCreateInfo) * queueInfoCount);
    for (uint32_t i = 0; i < queueFamilyCount; i++) {
        if (queueIndices[i] > 0) {
            queueInfo[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueInfo[i].pNext = VK_NULL_HANDLE;
            queueInfo[i].flags = 0;
            queueInfo[i].queueCount = queueIndices[i];
            queueInfo[i].queueFamilyIndex = i;
            queueInfo[i].pQueuePriorities = queuePriorities[i];
        }
    }

    // Extensions And Layers Are Not Really Useful Currently
    // In That Case We Can Initialize Them With Constant Data
    uint32_t extensionsCount = 1;
    const char* extensions[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    uint32_t layerCount = 0;
    const char** layers = VK_NULL_HANDLE;


    VkDevice device;
    VkDeviceCreateInfo deviceInfo = {
            .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            .pNext = VK_NULL_HANDLE,
            .flags = 0,
            .queueCreateInfoCount = queueInfoCount,
            .pQueueCreateInfos = queueInfo,
            .enabledLayerCount = layerCount,
            .ppEnabledLayerNames = layers,
            .enabledExtensionCount = extensionsCount,
            .ppEnabledExtensionNames = extensions,
            .pEnabledFeatures = &features,
    };
    vkCreateDevice(physicalDevice, &deviceInfo, VK_NULL_HANDLE, &device);

    // Get The Previously Initialized Queues Data And Actually Acquire The Queue Object
    for (uint32_t i = 0; i < queueRequestCount; i++) {
        VkQueue queue;
        vkGetDeviceQueue(device, (*queues)[i].familyIndex, (*queues)[i].queueIndex, &queue);
        (*queues)[i].queue = queue;
    }

    return device;
}
VkRenderPass PLCore_Priv_CreateRenderPass(VkDevice device, VkFormat presentableFormat, VkFormat depthFormat) {
    // Defines The Index And Type Of The Render Pass's Rendering Functionality
    VkAttachmentReference attachments[] = {
            {
                .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                .attachment = 0,
            },
            {
                .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                .attachment = 1,
            }
    };
    // Describes How The Attachment References Are Used More In Depth
    // Im Not Exactly Sure How The Render Pass Works So Keep That In Mind For My Descriptions
    VkAttachmentDescription description[] = {
            // The First Attachment Is For Rendering Color
            // We Take 1 Sample And Clear The Framebuffer Once We Get New Data
            // We Dont Care How The Data Is Stored, But We Do Want The Framebuffer To Presentable
            {
                .flags = 0,
                .format = presentableFormat,
                .samples = VK_SAMPLE_COUNT_1_BIT,
                .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
            },
            // The Second Attachment Is For Z Indexing (Depth)
            // We Take 1 Sample And Clear The Depth Buffer Once We Get New Data
            // We Dont Care How Data Is Stored, However We Do Need The Buffer To Be A Valid Depth Stencil For The Data
            {
                .flags = 0,
                .format = depthFormat,
                .samples = VK_SAMPLE_COUNT_1_BIT,
                .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
            }
    };
    // This Combines The 2 Attachments And Describes That We Want It To Be Used For Graphics
    VkSubpassDescription subpass = {
            .flags = 0,
            .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
            .inputAttachmentCount = 0,
            .pInputAttachments = VK_NULL_HANDLE,
            .colorAttachmentCount = 1,
            .pColorAttachments = &attachments[0],
            .pResolveAttachments = VK_NULL_HANDLE,
            .pDepthStencilAttachment = &attachments[1],
            .preserveAttachmentCount = 0,
    };
    // This Tells What The Subpass Is Dependant On Before The Final Result Is Presentable
    // (I Believe The Render Pass Will Wait On These Before Rendering)
    VkSubpassDependency dependency[] = {
            {
                .dependencyFlags = 0,
                .srcSubpass = VK_SUBPASS_EXTERNAL,
                .dstSubpass = 0,
                .srcAccessMask = 0,
                .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
                .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
                .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
            }
    };
    // This Combines The Subpass And Attachments Into 1 Object
    VkRenderPassCreateInfo renderPassInfo = {
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
            .pNext = VK_NULL_HANDLE,
            .flags = 0,
            .attachmentCount = 2,
            .pAttachments = description,
            .subpassCount = 1,
            .pSubpasses = &subpass,
            .dependencyCount = 1,
            .pDependencies = dependency,
    };
    VkRenderPass renderPass;
    vkCreateRenderPass(device, &renderPassInfo, VK_NULL_HANDLE, &renderPass);
    return renderPass;
}
VkCommandPool PLCore_Priv_CreateCommandPool(VkDevice device, uint32_t queueFamily, VkCommandPoolCreateFlagBits flags) {
    VkCommandPool pool;
    VkCommandPoolCreateInfo poolInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .pNext = VK_NULL_HANDLE,
            .flags = flags,
            .queueFamilyIndex = queueFamily,
    };
    vkCreateCommandPool(device, &poolInfo, VK_NULL_HANDLE, &pool);
    return pool;
}
VkCommandBuffer* PLCore_Priv_CreateCommandBuffers(VkDevice device, VkCommandPool pool, uint32_t count) {
    VkCommandBuffer* buffers = malloc(sizeof(VkCommandBuffer) * count);
    VkCommandBufferAllocateInfo allocInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .pNext = VK_NULL_HANDLE,
            .commandBufferCount = count,
            .commandPool = pool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
    };
    vkAllocateCommandBuffers(device, &allocInfo, buffers);
    return buffers;
}

GLFWwindow* PLCore_Priv_CreateWindow(VkInstance instance, uint32_t width, uint32_t height, VkSurfaceKHR* surface) {
    GLFWwindow* window = glfwCreateWindow(width, height, "Application", VK_NULL_HANDLE, VK_NULL_HANDLE);
    glfwCreateWindowSurface(instance, window, VK_NULL_HANDLE, surface);
    return window;
}

static VkPresentModeKHR findPresentMode(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) {
    VkPresentModeKHR PresentMode;
    uint32_t presentModeCount = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, VK_NULL_HANDLE);
    VkPresentModeKHR* presentModes = malloc(sizeof(VkPresentModeKHR) * presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, presentModes);
    for (uint32_t i = 0; i < presentModeCount; i++) {
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
static VkSurfaceFormatKHR findSurfaceFormat(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) {
    VkSurfaceFormatKHR SurfaceFormat;
    uint32_t surfaceFormatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &surfaceFormatCount, VK_NULL_HANDLE);
    VkSurfaceFormatKHR* surfaceFormats = malloc(sizeof(VkSurfaceFormatKHR) * surfaceFormatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &surfaceFormatCount, surfaceFormats);
    for (uint32_t i = 0; i < surfaceFormatCount; i++) {
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
static VkExtent2D getSurfaceExtent(GLFWwindow* window) {
    int x,y;
    glfwGetWindowSize(window, &x, &y);
    return (VkExtent2D) {.width = x, .height = y};
}

// TODO: Create Framebuffers

VkSwapchainKHR PLCore_Priv_CreateSwapchain(VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkExtent2D screenResolution, uint32_t queueFamily, VkSurfaceFormatKHR* surfaceFormat, VkPresentModeKHR* presentMode) {

    VkSwapchainKHR Swapchain;
    // Search Through All Available Formats And Select One
    *surfaceFormat = findSurfaceFormat(physicalDevice, surface);
    // Search Through All Available Present Modes And Select One
    // Select Mailbox If Possible Because That Is Best For The Engine
    *presentMode = findPresentMode(physicalDevice, surface);


    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    // Get The Capabilities Of The Graphics Card
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &surfaceCapabilities);

    // Figure Out The Minimum Number Of Images That Can Be Present In The Swapchain
    uint32_t minImageCount = surfaceCapabilities.minImageCount + 1;
    if (surfaceCapabilities.maxImageCount > 0 && minImageCount > surfaceCapabilities.maxImageCount) {
        minImageCount = surfaceCapabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR swapchainInfo;
    swapchainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainInfo.pNext = VK_NULL_HANDLE;
    swapchainInfo.flags = 0;
    swapchainInfo.surface = surface;
    swapchainInfo.minImageCount = minImageCount;
    swapchainInfo.imageFormat = (*surfaceFormat).format;
    swapchainInfo.imageColorSpace = (*surfaceFormat).colorSpace;
    swapchainInfo.imageExtent = screenResolution;
    swapchainInfo.imageArrayLayers = 1;
    swapchainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE; // Because Only One Queue Will Be Using The Image
    swapchainInfo.queueFamilyIndexCount = 1;
    swapchainInfo.pQueueFamilyIndices = &queueFamily;
    swapchainInfo.preTransform = surfaceCapabilities.currentTransform;
    swapchainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchainInfo.presentMode = *presentMode;
    swapchainInfo.clipped = VK_TRUE;
    swapchainInfo.oldSwapchain = VK_NULL_HANDLE;

    // Create Swapchain
    vkCreateSwapchainKHR(device, &swapchainInfo, VK_NULL_HANDLE, &Swapchain);

    return Swapchain;
}
VkImage* PLCore_Priv_AcquireSwapchainImages(VkDevice device, VkSwapchainKHR swapchain, VkFormat renderFormat, VkImageView** imageViews, uint32_t* imageCount) {
    vkGetSwapchainImagesKHR(device, swapchain, imageCount, VK_NULL_HANDLE);
    VkImage* images = malloc(sizeof(VkImage) * (*imageCount));
    *imageViews = malloc(sizeof(VkImage) * (*imageCount));
    vkGetSwapchainImagesKHR(device, swapchain, imageCount, images);
    for (int i = 0; i < *imageCount; i++) {
        VkImageViewCreateInfo viewInfo;
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.pNext = VK_NULL_HANDLE;
        viewInfo.flags = 0;
        viewInfo.image = images[i];
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = renderFormat;
        viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.layerCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseMipLevel = 0;
        vkCreateImageView(device, &viewInfo, VK_NULL_HANDLE, &((*imageViews)[i]));
    }
    return images;
}
VkImage PLCore_Priv_CreateDepthBuffer(VkDevice device, VkPhysicalDeviceMemoryProperties memoryProperties, VkImageView* depthView, VkFormat depthFormat, VkExtent2D resolution, uint32_t queueFamily) {
    // TODO: Automatically Find A Depth Format

    VkImageCreateInfo depthInfo;
    depthInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    depthInfo.pNext = VK_NULL_HANDLE;
    depthInfo.flags = 0;
    depthInfo.imageType = VK_IMAGE_TYPE_2D;
    depthInfo.format = depthFormat;
    depthInfo.extent = (VkExtent3D){.width = resolution.width, .height = resolution.height, 1};
    depthInfo.mipLevels = 1;
    depthInfo.arrayLayers = 1;
    depthInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    depthInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    depthInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    depthInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    depthInfo.queueFamilyIndexCount = 1;
    depthInfo.pQueueFamilyIndices = &queueFamily;
    depthInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;


    VkImage depthImage;
    VkDeviceMemory depthMemory;


    vkCreateImage(device, &depthInfo, VK_NULL_HANDLE, &depthImage);

    VkMemoryRequirements imageRequirements;
    vkGetImageMemoryRequirements(device, depthImage, &imageRequirements);

    VkMemoryAllocateInfo allocateInfo;
    allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocateInfo.pNext = VK_NULL_HANDLE;
    allocateInfo.allocationSize = imageRequirements.size;
    allocateInfo.memoryTypeIndex = findMemoryType(memoryProperties, imageRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    vkAllocateMemory(device, &allocateInfo, VK_NULL_HANDLE, &depthMemory);
    vkBindImageMemory(device, depthImage, depthMemory, 0);

    VkImageViewCreateInfo viewInfo;
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.pNext = VK_NULL_HANDLE;
    viewInfo.flags = 0;
    viewInfo.image = depthImage;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = depthFormat;
    viewInfo.components.r = VK_COMPONENT_SWIZZLE_R;
    viewInfo.components.g = VK_COMPONENT_SWIZZLE_G;
    viewInfo.components.b = VK_COMPONENT_SWIZZLE_B;
    viewInfo.components.a = VK_COMPONENT_SWIZZLE_A;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.layerCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;

    vkCreateImageView(device, &viewInfo, VK_NULL_HANDLE, &(*depthView));

    return depthImage;
}
VkFramebuffer PLCore_Priv_CreateFramebuffer(VkDevice device, VkExtent2D resolution, VkRenderPass renderPass, VkImageView swapchainImage, VkImageView depthView) {
    VkFramebuffer framebuffer;
    uint32_t attachmentCount = 2;
    VkImageView attachments[] = {swapchainImage, depthView};


    VkFramebufferCreateInfo bufferInfo;
    bufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    bufferInfo.pNext = VK_NULL_HANDLE;
    bufferInfo.flags = 0;
    bufferInfo.renderPass = renderPass;
    bufferInfo.attachmentCount = attachmentCount;
    bufferInfo.pAttachments = attachments;
    bufferInfo.width = resolution.width;
    bufferInfo.height = resolution.height;
    bufferInfo.layers = 1;

    vkCreateFramebuffer(device, &bufferInfo, VK_NULL_HANDLE, &framebuffer);
    return framebuffer;
}
PLCore_RenderInstance PLCore_CreateRenderingInstance() {
    PLCore_RenderInstance renderInstance;
    renderInstance.pl_instance.instance = PLCore_Priv_CreateInstance(&renderInstance.pl_instance.priv_Messenger);

    VkQueueFamilyProperties* queueFamilyProperties;
    uint32_t queueFamilyCount;

    renderInstance.pl_device.physicalDevice = PLCore_Priv_CreatePhysicalDevice(renderInstance.pl_instance.instance, &queueFamilyCount, &queueFamilyProperties);
    vkGetPhysicalDeviceProperties(renderInstance.pl_device.physicalDevice, &renderInstance.pl_device.properties);
    vkGetPhysicalDeviceFeatures(renderInstance.pl_device.physicalDevice, &renderInstance.pl_device.features);
    vkGetPhysicalDeviceMemoryProperties(renderInstance.pl_device.physicalDevice, &renderInstance.pl_device.memoryProperties);

    PLCore_DeviceQueue* queues = VK_NULL_HANDLE;
    uint32_t queueRequestCount = 2;
    VkQueueFlagBits queueRequest[] = {VK_QUEUE_GRAPHICS_BIT, VK_QUEUE_TRANSFER_BIT};
    renderInstance.pl_device.device = PLCore_Priv_CreateDevice(renderInstance.pl_device.physicalDevice, queueFamilyCount, queueFamilyProperties, renderInstance.pl_device.features, queueRequestCount, queueRequest, &queues);

    renderInstance.pl_device.graphicsQueue = queues[0];
    renderInstance.pl_device.transferQueue = queues[1];

    renderInstance.transferPool.pool = PLCore_Priv_CreateCommandPool(renderInstance.pl_device.device, renderInstance.pl_device.transferQueue.familyIndex, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

    return renderInstance;
}
PLCore_Window PLCore_CreateWindow(VkInstance instance, uint32_t width, uint32_t height) {
    PLCore_Window window;
    window.window = PLCore_Priv_CreateWindow(instance, width, height, &window.surface);
    window.resolution = (VkExtent2D){.width = width, .height = height};
    window.viewport = (VkViewport){
            .width = (float)width,
            .height = (float)height,
            .x = 0,
            .y = 0,
            .maxDepth = 1.0f,
            .minDepth = 0.0f,
    };
    window.renderArea = (VkRect2D){
        .extent = window.resolution,
        .offset.x = 0,
        .offset.y = 0,
    };
    return window;
}
PLCore_Renderer PLCore_CreateRenderer(PLCore_RenderInstance instance, PLCore_Window window) {
    PLCore_Renderer renderer;

    VkSurfaceFormatKHR renderFormat;
    VkPresentModeKHR renderMode;
    renderer.swapchain = PLCore_Priv_CreateSwapchain(instance.pl_device.device, instance.pl_device.physicalDevice, window.surface, window.resolution, instance.pl_device.graphicsQueue.familyIndex, &renderFormat, &renderMode);
    renderer.swapchainImages = PLCore_Priv_AcquireSwapchainImages(instance.pl_device.device, renderer.swapchain, renderFormat.format, &renderer.swapchainImageViews, &renderer.swapchainImageCount);

    VkFormat depthFormat = VK_FORMAT_D32_SFLOAT;
    PLCore_Image depthImage;
    depthImage.image = PLCore_Priv_CreateDepthBuffer(instance.pl_device.device, instance.pl_device.memoryProperties, &depthImage.view, depthFormat, window.resolution, instance.pl_device.graphicsQueue.familyIndex);
    renderer.pl_depthImage = depthImage;

    renderer.renderPass = PLCore_Priv_CreateRenderPass(instance.pl_device.device, renderFormat.format, depthFormat);

    renderer.framebuffers = malloc(sizeof(VkFramebuffer) * renderer.swapchainImageCount);
    for (uint32_t i = 0; i < renderer.swapchainImageCount; i++) {
        renderer.framebuffers[i] = PLCore_Priv_CreateFramebuffer(instance.pl_device.device, window.resolution, renderer.renderPass, renderer.swapchainImageViews[i], renderer.pl_depthImage.view);
    }

    renderer.graphicsPool.pool = PLCore_Priv_CreateCommandPool(instance.pl_device.device, instance.pl_device.graphicsQueue.familyIndex, 0);
    renderer.graphicsPool.buffers = PLCore_Priv_CreateCommandBuffers(instance.pl_device.device, renderer.graphicsPool.pool, renderer.swapchainImageCount);

    return renderer;
}