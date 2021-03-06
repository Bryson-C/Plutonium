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


PLCore_ShaderModule                             PLCore_Priv_CreateShader(VkDevice device, const char* path, const char* entryPoint) {
    PLCore_ShaderModule shader;
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
VkPipelineShaderStageCreateInfo                 PLCore_Priv_CreateShaderStage(PLCore_ShaderModule shader, VkShaderStageFlagBits shaderStage) {
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
VkPipelineInputAssemblyStateCreateInfo          PLCore_Priv_CreateInputAssemblyStage(VkPrimitiveTopology topology) {
    VkPipelineInputAssemblyStateCreateInfo inputAssembly;
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.pNext = VK_NULL_HANDLE;
    inputAssembly.flags = 0;
    inputAssembly.topology = topology;
    inputAssembly.primitiveRestartEnable = VK_FALSE;
    return inputAssembly;
}
VkVertexInputAttributeDescription               PLCore_Priv_CreateVertexAttribute(uint32_t binding, uint32_t location, VkFormat format, uint32_t offset) {
    VkVertexInputAttributeDescription attribute;
    attribute.binding = binding;
    attribute.location = location;
    attribute.format = format;
    attribute.offset = offset;
    return attribute;
}
VkVertexInputBindingDescription                 PLCore_Priv_CreateVertexBinding(uint32_t binding, VkVertexInputRate inputRate, VkDeviceSize stride) {
    VkVertexInputBindingDescription description;
    description.binding = binding;
    description.inputRate = inputRate;
    description.stride = stride;
    return description;
}
VkPipelineVertexInputStateCreateInfo            PLCore_Priv_CreateVertexInput(uint32_t attributeCount, VkVertexInputAttributeDescription* attributes, uint32_t bindingCount, VkVertexInputBindingDescription* bindings) {

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
VkViewport                                      PLCore_Priv_CreateViewport(VkExtent2D extent) {
    VkViewport viewport;
    viewport.x = 0;
    viewport.y = 0;
    viewport.width = (float)extent.width;
    viewport.height = (float)extent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    return viewport;
}
VkRect2D                                        PLCore_Priv_CreateScissor(VkExtent2D extent) {
    VkRect2D scissor;
    scissor.extent = extent;
    scissor.offset = (VkOffset2D){.x = 0, .y = 0};
    return scissor;
}
VkPipelineViewportStateCreateInfo               PLCore_Priv_CreateViewportState(VkViewport viewport, VkRect2D scissor) {
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
VkPipelineLayout                                PLCore_Priv_CreatePipelineLayout(VkDevice device, uint32_t pushConstantCount, VkPushConstantRange* pushConstants, uint32_t setLayoutCount, VkDescriptorSetLayout* setLayouts) {
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
VkPipelineRasterizationStateCreateInfo          PLCore_Priv_CreateRasterizer(VkPolygonMode polygonMode, VkCullModeFlagBits cullMode, VkFrontFace frontFace, float lineWidth) {
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
VkPipelineMultisampleStateCreateInfo            PLCore_Priv_CreateMultisampleState() {
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
VkPipelineColorBlendAttachmentState             PLCore_Priv_CreateColorBlendAttachment() {
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
VkPipelineColorBlendStateCreateInfo             PLCore_Priv_CreateColorBlend(uint32_t attachmentCount, VkPipelineColorBlendAttachmentState attachment) {
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
VkPipelineDepthStencilStateCreateInfo           PLCore_Priv_CreateDepthStencilState() {
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

PLCore_PipelineBuilder PLCore_Priv_CreateBlankPipelineBuilder() {
    // Initializes A Blank Builder
    PLCore_PipelineBuilder builder;
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
    builder.hasViewport = 0;
    return builder;
}

void PLCore_Priv_AddShadersToPipelineBuilder(PLCore_PipelineBuilder* builder, uint32_t shaderCount, VkPipelineShaderStageCreateInfo* shaders) {
    builder->hasShaders = 1;
    builder->shaderStages = shaders;
    builder->shaderStageCount = shaderCount;
}
void PLCore_Priv_AddVertexInputToPipelineBuilder(PLCore_PipelineBuilder* builder, VkPipelineVertexInputStateCreateInfo vertexInput) {
    builder->hasVertexInput = 1;
    builder->vertexInput = vertexInput;
}
void PLCore_Priv_AddInputAssemblyToPipelineBuilder(PLCore_PipelineBuilder* builder, VkPipelineInputAssemblyStateCreateInfo inputAssembly) {
    builder->hasInputAssembly = 1;
    builder->inputAssembly = inputAssembly;
}
void PLCore_Priv_AddViewportStateToPipelineBuilder(PLCore_PipelineBuilder* builder, VkPipelineViewportStateCreateInfo viewportState) {
    builder->hasViewportState = 1;
    builder->viewportState = viewportState;
}
void PLCore_Priv_AddPipelineLayoutToPipelineBuilder(PLCore_PipelineBuilder* builder, VkPipelineLayout pipelineLayout) {
    builder->hasPipelineLayout = 1;
    builder->pipelineLayout = pipelineLayout;
}
void PLCore_Priv_AddRasterizerToPipelineBuilder(PLCore_PipelineBuilder* builder, VkPipelineRasterizationStateCreateInfo rasterizer) {
    builder->hasRasterizer = 1;
    builder->rasterizer = rasterizer;
}
void PLCore_Priv_AddMultisampleStateToPipelineBuilder(PLCore_PipelineBuilder* builder, VkPipelineMultisampleStateCreateInfo multisample) {
    builder->hasMultisample = 1;
    builder->multisample = multisample;
}
void PLCore_Priv_AddColorBlendStateToPipelineBuilder(PLCore_PipelineBuilder* builder, VkPipelineColorBlendStateCreateInfo colorBlend) {
    builder->hasColorBlend = 1;
    builder->colorBlend = colorBlend;
}
void PLCore_Priv_AddRenderPassToPipelineBuilder(PLCore_PipelineBuilder* builder, VkRenderPass renderPass) {
    builder->hasRenderPass = 1;
    builder->renderPass = renderPass;
}
void PLCore_Priv_AddDepthStencilToPipelineBuilder(PLCore_PipelineBuilder* builder, VkPipelineDepthStencilStateCreateInfo depthStencil) {
    builder->hasDepthStencil = 1;
    builder->depthStencil = depthStencil;
}

VkPipeline PLCore_Priv_CreatePipelineFromBuilder(VkDevice device, PLCore_PipelineBuilder* builder, VkPipelineLayout* layout) {
    VkPipeline pipeline;

    // Cannot Go Without Shaders
    if (builder->hasShaders == 0) {}
    if (builder->hasVertexInput == 0) {}

    if (builder->hasDepthStencil == 0) { PLCore_Priv_AddDepthStencilToPipelineBuilder(builder, PLCore_Priv_CreateDepthStencilState()); }
    if (builder->hasInputAssembly == 0) { PLCore_Priv_AddInputAssemblyToPipelineBuilder(builder, PLCore_Priv_CreateInputAssemblyStage(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)); }
    if (builder->hasPipelineLayout == 0) { PLCore_Priv_AddPipelineLayoutToPipelineBuilder(builder, PLCore_Priv_CreatePipelineLayout(device, 0, VK_NULL_HANDLE, 0, VK_NULL_HANDLE)); }
    if (builder->hasMultisample == 0) { PLCore_Priv_AddMultisampleStateToPipelineBuilder(builder, PLCore_Priv_CreateMultisampleState()); }
    if (builder->hasColorBlend == 0) { PLCore_Priv_AddColorBlendStateToPipelineBuilder(builder, PLCore_Priv_CreateColorBlend(1, PLCore_Priv_CreateColorBlendAttachment())); }
    if (builder->hasRasterizer == 0) { PLCore_Priv_AddRasterizerToPipelineBuilder(builder, PLCore_Priv_CreateRasterizer(VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_CLOCKWISE, 1.0f)); }

    // Cannot Go Without Viewport or Extent
    if (builder->hasViewportState == 0) {
        fprintf(stderr,"Invalid Or No Viewport Present In Pipeline Builder Therefore Will Not Continue");
        assert(1);
    }

    if (builder->hasRenderPass == 0) {
        fprintf(stderr,"Invalid Or No Render Pass Present In Pipeline Builder Therefore Will Not Continue");
        assert(1);
    }


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
    pipelineInfo.renderPass = builder->renderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.basePipelineIndex = -1;

    *layout = builder->pipelineLayout;
    vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, VK_NULL_HANDLE, &pipeline);
    return pipeline;
}

VkBuffer PLCore_Priv_CreateBuffer(VkDevice device, VkPhysicalDeviceMemoryProperties memoryProperties, VkDeviceSize size, uint32_t queueFamily, VkBufferUsageFlagBits usage, VkMemoryPropertyFlagBits memoryFlags, VkDeviceMemory* memory) {
    VkBuffer buffer;

    VkBufferCreateInfo bufferInfo;
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.pNext = VK_NULL_HANDLE;
    bufferInfo.flags = 0;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    bufferInfo.queueFamilyIndexCount = 1;
    bufferInfo.pQueueFamilyIndices = &queueFamily;

    // Create Buffer
    vkCreateBuffer(device, &bufferInfo, VK_NULL_HANDLE, &buffer);

    // Ask For Allocation Requirements
    VkMemoryRequirements bufferRequirements;
    vkGetBufferMemoryRequirements(device, buffer, &bufferRequirements);

    VkMemoryAllocateInfo bufferAlloc;
    bufferAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    bufferAlloc.pNext = VK_NULL_HANDLE;
    bufferAlloc.allocationSize = bufferRequirements.size;
    bufferAlloc.memoryTypeIndex = findMemoryType(memoryProperties,bufferRequirements.memoryTypeBits, memoryFlags);

    // Allocate And Bind Memory For Buffer Based On What Is Needed
    vkAllocateMemory(device, &bufferAlloc, VK_NULL_HANDLE, memory);
    vkBindBufferMemory(device, buffer, *memory, 0);

    return buffer;
}
VkBuffer PLCore_Priv_CreateGPUBuffer(VkDevice device, VkPhysicalDeviceMemoryProperties memoryProperties, uint32_t queueFamily, VkDeviceSize size, VkBufferUsageFlagBits usageFlags, VkCommandBuffer cmdBuffer, VkQueue submitQueue, void* data, VkDeviceMemory* memory) {
    // Staging Buffer Will Store The Data
    // Next The Staging Buffer Will Transfer It To A Buffer Which Is Allocated To The GPU
    VkDeviceMemory stagingMemory;
    VkBuffer stagingBuffer = PLCore_Priv_CreateBuffer(device, memoryProperties, size, queueFamily, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, CPU_COHERENT | CPU_VISIBLE, &stagingMemory);
    // Buffer Is Created For The GPU
    // It Will Accept Data From The Staging Buffer
    VkDeviceMemory bufferMemory;
    VkBuffer buffer = PLCore_Priv_CreateBuffer(device, memoryProperties, size, queueFamily, usageFlags | VK_BUFFER_USAGE_TRANSFER_DST_BIT, GPU_LOCAL, &bufferMemory);

    // Allocate Data To The Staging Buffer
    void* bufferData;
    vkMapMemory(device, stagingMemory, 0, size, 0, &bufferData);
    memcpy(bufferData, data, (VkDeviceSize) size);
    vkUnmapMemory(device, stagingMemory);

    // Copying The Data From The Staging Buffer To The GPU Allocated Buffer
    VkCommandBufferBeginInfo vertexCopyBeginfo;
    vertexCopyBeginfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    vertexCopyBeginfo.pNext = VK_NULL_HANDLE;
    vertexCopyBeginfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vertexCopyBeginfo.pInheritanceInfo = VK_NULL_HANDLE;

    vkBeginCommandBuffer(cmdBuffer, &vertexCopyBeginfo);
    VkBufferCopy CopyInfo = {.size = size,.dstOffset = 0,.srcOffset = 0};
    vkCmdCopyBuffer(cmdBuffer, stagingBuffer, buffer, 1, &CopyInfo);
    vkEndCommandBuffer(cmdBuffer);

    // Submit The Data To Be Transfered To The GPU Allocated Buffer
    VkSubmitInfo submitInfo = {
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .pNext = VK_NULL_HANDLE,
            .waitSemaphoreCount = 0,
            .pWaitSemaphores = VK_NULL_HANDLE,
            .pWaitDstStageMask = VK_NULL_HANDLE,
            .commandBufferCount = 1,
            .pCommandBuffers = &cmdBuffer,
            .signalSemaphoreCount = 0,
            .pSignalSemaphores = VK_NULL_HANDLE,
    };

    // Fence To Wait For The End Of The Submit Task
    VkFenceCreateInfo fenceInfo = {
            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
            .pNext = VK_NULL_HANDLE,
            .flags = 0,
    };
    VkFence fence;
    vkCreateFence(device, &fenceInfo, VK_NULL_HANDLE, &fence);

    vkQueueSubmit(submitQueue, 1, &submitInfo, fence);
    vkWaitForFences(device, 1, &fence, VK_TRUE, UINT64_MAX);

    // We Dont Need The Staging Buffer Becuase It Gave Up Its Data To The GPU Buffer
    vkFreeMemory(device, stagingMemory, VK_NULL_HANDLE);
    vkDestroyBuffer(device, stagingBuffer, VK_NULL_HANDLE);

    return buffer;
}
void PLCore_UploadDataToBuffer(VkDevice device, VkDeviceMemory* memory, VkDeviceSize size, void* data) {
    void* bufferData;
    vkMapMemory(device, *memory, 0, size, 0, &bufferData);
    memcpy(bufferData, data, (VkDeviceSize)size);
    vkUnmapMemory(device, *memory);
}

static void BeginFrameRecording(VkCommandBuffer buffer, VkRenderPass renderPass, VkFramebuffer framebuffer, VkRect2D scissor, uint32_t clearCount, VkClearValue* clears) {
    VkRenderPassBeginInfo beginfo;
    beginfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    beginfo.pNext = VK_NULL_HANDLE;
    beginfo.renderPass = renderPass;
    beginfo.framebuffer = framebuffer;
    beginfo.renderArea = scissor;
    beginfo.clearValueCount = clearCount;
    beginfo.pClearValues = clears;

    VkCommandBufferBeginInfo cmdBeginfo;
    cmdBeginfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    cmdBeginfo.pNext = VK_NULL_HANDLE;
    cmdBeginfo.flags = 0;
    cmdBeginfo.pInheritanceInfo = VK_NULL_HANDLE;

    vkBeginCommandBuffer(buffer, &cmdBeginfo);
    vkCmdBeginRenderPass(buffer, &beginfo, VK_SUBPASS_CONTENTS_INLINE);
}
static void EndFrameRecording(VkCommandBuffer buffer) {
    vkCmdEndRenderPass(buffer);
    vkEndCommandBuffer(buffer);
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

    renderInstance.pl_transferPool.pool = PLCore_Priv_CreateCommandPool(renderInstance.pl_device.device, renderInstance.pl_device.transferQueue.familyIndex, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

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

    renderer.backBuffers = 2;
    renderer.priv_activeFrame = 0;
    renderer.priv_renderFences = malloc(sizeof(VkFence) * renderer.backBuffers);
    renderer.priv_signalSemahores = malloc(sizeof(VkSemaphore) * renderer.backBuffers);
    renderer.priv_waitSemahores = malloc(sizeof(VkSemaphore) * renderer.backBuffers);
    for (uint32_t i = 0; i < renderer.backBuffers; i++) {
        VkFenceCreateInfo fenceInfo = {
                .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
                .pNext = VK_NULL_HANDLE,
                .flags = VK_FENCE_CREATE_SIGNALED_BIT,
        };
        VkSemaphoreCreateInfo semaInfo = {
                .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
                .pNext = VK_NULL_HANDLE,
                .flags = 0,
        };
        vkCreateFence(instance.pl_device.device, &fenceInfo, VK_NULL_HANDLE, &renderer.priv_renderFences[i]);
        vkCreateSemaphore(instance.pl_device.device, &semaInfo, VK_NULL_HANDLE, &renderer.priv_signalSemahores[i]);
        vkCreateSemaphore(instance.pl_device.device, &semaInfo, VK_NULL_HANDLE, &renderer.priv_waitSemahores[i]);
    }


    return renderer;
}
PLCore_GraphicsPipeline PLCore_CreatePipeline(PLCore_RenderInstance instance, PLCore_Renderer renderer, PLCore_Window window, VkPipelineVertexInputStateCreateInfo vertexInput, PLCore_ShaderModule vertexShader, PLCore_ShaderModule fragmentShader) {
    PLCore_GraphicsPipeline pipeline;
    PLCore_PipelineBuilder builder = PLCore_Priv_CreateBlankPipelineBuilder();

    PLCore_Priv_AddRenderPassToPipelineBuilder(&builder, renderer.renderPass);

    VkPipelineShaderStageCreateInfo shaders[] = {
            PLCore_Priv_CreateShaderStage(vertexShader, VK_SHADER_STAGE_VERTEX_BIT),
            PLCore_Priv_CreateShaderStage(fragmentShader, VK_SHADER_STAGE_FRAGMENT_BIT),
    };
    PLCore_Priv_AddShadersToPipelineBuilder(&builder, 2, shaders);

    VkPipelineViewportStateCreateInfo viewport = PLCore_Priv_CreateViewportState(window.viewport, window.renderArea);
    PLCore_Priv_AddViewportStateToPipelineBuilder(&builder, viewport);

    PLCore_Priv_AddVertexInputToPipelineBuilder(&builder, vertexInput);

    pipeline.pl_builder = builder;
    pipeline.pipeline = PLCore_Priv_CreatePipelineFromBuilder(instance.pl_device.device, &builder, &pipeline.layout);
    return pipeline;
}
PLCore_Buffer PLCore_CreateBuffer(PLCore_RenderInstance instance, VkDeviceSize size, VkBufferUsageFlagBits usage) {
    VkDeviceMemory memory;
    VkBuffer buffer = PLCore_Priv_CreateBuffer(instance.pl_device.device, instance.pl_device.memoryProperties, size, instance.pl_device.transferQueue.familyIndex, usage, CPU_VISIBLE, &memory);
    PLCore_Buffer pl_buffer = {
            .buffer = buffer,
            .memory = memory
    };
    return pl_buffer;
}
PLCore_Buffer PLCore_CreateGPUBuffer(PLCore_RenderInstance instance, VkDeviceSize size, VkBufferUsageFlagBits usage, void* data) {
    VkCommandBuffer cmdBuffer = PLCore_Priv_CreateCommandBuffers(instance.pl_device.device, instance.pl_transferPool.pool, 1)[0];
    VkDeviceMemory memory;
    VkBuffer buffer = PLCore_Priv_CreateGPUBuffer(instance.pl_device.device, instance.pl_device.memoryProperties, instance.pl_device.transferQueue.familyIndex, size, usage, cmdBuffer, instance.pl_device.transferQueue.queue, data, &memory);
    PLCore_Buffer pl_buffer = {
            .buffer = buffer,
            .memory = memory,
    };
    vkFreeCommandBuffers(instance.pl_device.device, instance.pl_transferPool.pool, 1, &cmdBuffer);
    return pl_buffer;
}

void PLCore_BeginFrame(PLCore_RenderInstance instance, PLCore_Renderer renderer, PLCore_Window window) {
    VkResult imageAcquireResult = vkAcquireNextImageKHR(instance.pl_device.device, renderer.swapchain, UINT64_MAX, renderer.priv_waitSemahores[renderer.priv_activeFrame], VK_NULL_HANDLE, &renderer.priv_imageIndex);
    if (imageAcquireResult == VK_ERROR_OUT_OF_DATE_KHR) {
        // TODO: Rebuilding Swapchain
        //DestroyRenderStateRenderer();
        //CreateRenderStateRenderer();
        fprintf(stderr, "Rendering Objects Need To Be Rebuilt\n");
        assert(1);
        return;
    }

    vkWaitForFences(instance.pl_device.device, 1, &renderer.priv_renderFences[renderer.priv_activeFrame], VK_TRUE, UINT64_MAX);
    vkResetFences(instance.pl_device.device, 1, &renderer.priv_renderFences[renderer.priv_activeFrame]);


    VkRect2D scissor = window.renderArea;
    VkClearValue clears[2];
    clears[0].color.float32[0] = 0.015f;
    clears[0].color.float32[1] = 0.015f;
    clears[0].color.float32[2] = 0.015f;
    clears[0].color.float32[3] = 1.0f;
    clears[1].depthStencil.depth = 1.0f;
    clears[1].depthStencil.stencil = 0;

    BeginFrameRecording(renderer.graphicsPool.buffers[renderer.priv_imageIndex], renderer.renderPass, renderer.framebuffers[renderer.priv_imageIndex], scissor, 2, clears);
}
void PLCore_EndFrame(PLCore_RenderInstance instance, PLCore_Renderer renderer) {
    EndFrameRecording(renderer.graphicsPool.buffers[renderer.priv_imageIndex]);

    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

    VkSubmitInfo submitInfo;
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pNext = VK_NULL_HANDLE;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &renderer.priv_waitSemahores[renderer.priv_activeFrame];
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &renderer.graphicsPool.buffers[renderer.priv_imageIndex];
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &renderer.priv_signalSemahores[renderer.priv_imageIndex];


    vkQueueSubmit(instance.pl_device.graphicsQueue.queue, 1, &submitInfo, renderer.priv_renderFences[renderer.priv_activeFrame]);

    VkResult presentationResult;
    VkPresentInfoKHR presentInfo;
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.pNext = VK_NULL_HANDLE;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &renderer.priv_signalSemahores[renderer.priv_activeFrame];
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &renderer.swapchain;
    presentInfo.pImageIndices = &renderer.priv_imageIndex;
    presentInfo.pResults = &presentationResult;

    VkResult renderResult = vkQueuePresentKHR(instance.pl_device.graphicsQueue.queue, &presentInfo);
    if (renderResult == VK_ERROR_OUT_OF_DATE_KHR || renderResult == VK_SUBOPTIMAL_KHR) {
        // TODO: Rebuilding Swapchain
        //DestroyRenderStateRenderer();
        //CreateRenderStateRenderer();
        fprintf(stderr, "Rendering Objects Need To Be Rebuilt\n");
        assert(1);
        return;
    }

    renderer.priv_activeFrame = (renderer.priv_activeFrame+1)%renderer.backBuffers;
}

VkCommandBuffer PLCore_ActiveRenderBuffer(PLCore_Renderer renderer) {
    return renderer.graphicsPool.buffers[renderer.priv_imageIndex];
}

