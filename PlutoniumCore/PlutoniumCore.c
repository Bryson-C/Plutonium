//
// Created by Owner on 6/23/2022.
//

#include "PlutoniumCore.h"
#include "../../stb_image.h"


static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback( VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
    if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) printf("[Vk]: %s\n\n",pCallbackData->pMessage);
    return VK_FALSE;
}
static uint32_t findMemoryType(VkPhysicalDeviceMemoryProperties memoryProperties, uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++)
        if ((typeFilter & (1 << i)) && (memoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
            return i;
}



// TODO: Remove Validation Layers For Release Builds
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
                .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
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

    uint32_t monitorCount = 0;
    GLFWmonitor** monitors = glfwGetMonitors(&monitorCount);
    GLFWmonitor* monitor = (monitorCount > 0) ? monitors[0] : VK_NULL_HANDLE;

    int32_t monitorWidth, monitorHeight;
    glfwGetMonitorPhysicalSize(monitor, &monitorWidth, &monitorHeight);

    glfwSetWindowPos(window, (monitorWidth/2) + (int_fast32_t)(width/2), (monitorHeight/2));

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


PLCore_ShaderModule PLCore_CreateShader(PLCore_RenderInstance instance, const char* path, VkShaderStageFlagBits stage, const char* entryPoint, VkDescriptorPool allocationPool) {
    PLCore_ShaderModule shader;
    shader.path = path;
    shader.entryPoint = entryPoint;
    shader.stage = stage;

    fopen_s(&shader.file, path, "rb");
    fseek(shader.file, 0, SEEK_END);
    shader.size = ftell(shader.file);
    rewind(shader.file);

    shader.buffer = malloc(sizeof(char) * shader.size + 1);
    fread_s(shader.buffer, shader.size, sizeof(char), shader.size, shader.file);
    shader.buffer[shader.size] = '\0';

    VkShaderModuleCreateInfo shaderInfo;
    shaderInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shaderInfo.pNext = VK_NULL_HANDLE;
    shaderInfo.flags = 0;
    shaderInfo.codeSize = shader.size;
    shaderInfo.pCode = (uint32_t*)shader.buffer;

    shader.result = vkCreateShaderModule(instance.pl_device.device, &shaderInfo, VK_NULL_HANDLE, &shader.module);

    shader.descriptorSets = PLCore_CreateDescriptorSetFromShader(instance, shader, allocationPool, &shader.descriptorSetCount);

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

    // Creates A Template Viewport
    VkPipelineViewportStateCreateInfo viewport = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
            .pNext = VK_NULL_HANDLE,
            .flags = 0,
            .viewportCount = 1,
            .pViewports = VK_NULL_HANDLE,
            .scissorCount = 1,
            .pScissors = VK_NULL_HANDLE,
    };

    if (builder->hasRenderPass == 0) {
        fprintf(stderr,"Invalid Or No Render Pass Present In Pipeline Builder Therefore Will Not Continue");
        assert(1);
    }


    VkDynamicState dynamicStates[] = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR,
    };

    // Asks For A Dynamic Viewport And RenderArea(Scissor)
    VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
            .pNext = VK_NULL_HANDLE,
            .flags = 0,
            .dynamicStateCount = 2,
            .pDynamicStates = dynamicStates,
    };


    VkGraphicsPipelineCreateInfo pipelineInfo;
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.pNext = VK_NULL_HANDLE;
    pipelineInfo.flags = 0;
    pipelineInfo.stageCount = builder->shaderStageCount;
    pipelineInfo.pStages = builder->shaderStages;
    pipelineInfo.pVertexInputState = &builder->vertexInput;
    pipelineInfo.pInputAssemblyState = &builder->inputAssembly;
    pipelineInfo.pTessellationState = VK_NULL_HANDLE;
    pipelineInfo.pViewportState = &viewport;
    pipelineInfo.pRasterizationState = &builder->rasterizer;
    pipelineInfo.pMultisampleState = &builder->multisample;
    pipelineInfo.pDepthStencilState = &builder->depthStencil;
    pipelineInfo.pColorBlendState = &builder->colorBlend;
    pipelineInfo.pDynamicState = &dynamicStateCreateInfo;
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
PLCore_Window PLCore_CreateWindow(PLCore_RenderInstance instance, uint32_t width, uint32_t height) {
    PLCore_Window window;
    window.window = PLCore_Priv_CreateWindow(instance.pl_instance.instance, width, height, &window.surface);
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
static void PLCore_ResizeWindow(PLCore_RenderInstance instance, PLCore_Window* window) {
    int width, height;
    glfwGetWindowSize((*window).window, &width, &height);

    glfwCreateWindowSurface(instance.pl_instance.instance, window->window, VK_NULL_HANDLE, &((*window).surface));
    (*window).resolution = (VkExtent2D){.width = width, .height = height};
    (*window).viewport = (VkViewport){
            .width = (float)width,
            .height = (float)height,
            .x = 0,
            .y = 0,
            .maxDepth = 1.0f,
            .minDepth = 0.0f,
    };
    (*window).renderArea = (VkRect2D){
            .extent = window->resolution,
            .offset.x = 0,
            .offset.y = 0,
    };
}
PLCore_Renderer PLCore_CreateRenderer(PLCore_RenderInstance instance, PLCore_Window window) {
    PLCore_Renderer renderer;

    VkBool32 presentableQueue;
    vkGetPhysicalDeviceSurfaceSupportKHR(instance.pl_device.physicalDevice, instance.pl_device.graphicsQueue.familyIndex, window.surface, &presentableQueue);
    //printf("Presentable: %s\n", (presentableQueue) ? "True" : "False");

    VkSurfaceFormatKHR renderFormat;
    VkPresentModeKHR renderMode;
    renderer.swapchain = PLCore_Priv_CreateSwapchain(instance.pl_device.device, instance.pl_device.physicalDevice, window.surface, window.resolution, instance.pl_device.graphicsQueue.familyIndex, &renderFormat, &renderMode);
    renderer.swapchainImages = PLCore_Priv_AcquireSwapchainImages(instance.pl_device.device, renderer.swapchain, renderFormat.format, &renderer.swapchainImageViews, &renderer.swapchainImageCount);

    VkFormat depthFormat = VK_FORMAT_D32_SFLOAT;
    PLCore_Image depthImage;
    depthImage.image = PLCore_Priv_CreateDepthBuffer(instance.pl_device.device, instance.pl_device.memoryProperties, &depthImage.view, depthFormat, window.resolution, instance.pl_device.graphicsQueue.familyIndex);
    renderer.pl_depthImage = depthImage;
    renderer.pl_depthImage.requiredSize = 0;

    renderer.renderPass = PLCore_Priv_CreateRenderPass(instance.pl_device.device, renderFormat.format, depthFormat);

    renderer.framebuffers = malloc(sizeof(VkFramebuffer) * renderer.swapchainImageCount);
    for (uint32_t i = 0; i < renderer.swapchainImageCount; i++) {
        renderer.framebuffers[i] = PLCore_Priv_CreateFramebuffer(instance.pl_device.device, window.resolution, renderer.renderPass, renderer.swapchainImageViews[i], renderer.pl_depthImage.view);
    }

    renderer.graphicsPool.pool = PLCore_Priv_CreateCommandPool(instance.pl_device.device, instance.pl_device.graphicsQueue.familyIndex, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    renderer.graphicsPool.buffers = PLCore_Priv_CreateCommandBuffers(instance.pl_device.device, renderer.graphicsPool.pool, renderer.swapchainImageCount);

    renderer.backBuffers = 2;
    renderer.priv_activeFrame = 0;
    renderer.priv_renderFences = malloc(sizeof(VkFence) * renderer.backBuffers);
    renderer.priv_signalSemaphores = malloc(sizeof(VkSemaphore) * renderer.backBuffers);
    renderer.priv_waitSemaphores = malloc(sizeof(VkSemaphore) * renderer.backBuffers);
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
        vkCreateSemaphore(instance.pl_device.device, &semaInfo, VK_NULL_HANDLE, &renderer.priv_signalSemaphores[i]);
        vkCreateSemaphore(instance.pl_device.device, &semaInfo, VK_NULL_HANDLE, &renderer.priv_waitSemaphores[i]);
    }


    return renderer;
}
PLCore_GraphicsPipeline PLCore_CreatePipeline(PLCore_RenderInstance instance, PLCore_Renderer renderer, VkPipelineVertexInputStateCreateInfo vertexInput, PLCore_ShaderModule vertexShader, PLCore_ShaderModule fragmentShader, VkPipelineLayout* layout) {
    PLCore_GraphicsPipeline pipeline;
    printf("\n -- Unused Field In PLCore_GraphicsPipeline, Initialized Field As NULL At Line %u\n\n", __LINE__ + 1);
    pipeline.descriptorSets = VK_NULL_HANDLE;
    PLCore_PipelineBuilder builder = PLCore_Priv_CreateBlankPipelineBuilder();

    VkPipelineShaderStageCreateInfo shaders[] = {
            PLCore_Priv_CreateShaderStage(vertexShader, VK_SHADER_STAGE_VERTEX_BIT),
            PLCore_Priv_CreateShaderStage(fragmentShader, VK_SHADER_STAGE_FRAGMENT_BIT),
    };
    PLCore_ShaderModule shaderModules[] = {
            vertexShader,
            fragmentShader
    };

    if (layout == VK_NULL_HANDLE)
        pipeline.layout = PLCore_Priv_CreatePipelineLayout(instance.pl_device.device, 0, VK_NULL_HANDLE, 0, VK_NULL_HANDLE);
    else
        pipeline.layout = *layout;

    PLCore_Priv_AddShadersToPipelineBuilder(&builder, 2, shaders);
    PLCore_Priv_AddRenderPassToPipelineBuilder(&builder, renderer.renderPass);
    PLCore_Priv_AddVertexInputToPipelineBuilder(&builder, vertexInput);
    PLCore_Priv_AddPipelineLayoutToPipelineBuilder(&builder, pipeline.layout);

    pipeline.pipeline = PLCore_Priv_CreatePipelineFromBuilder(instance.pl_device.device, &builder, &pipeline.layout);
    return pipeline;
}


static void PLCore_DestroyRenderObjects(PLCore_RenderInstance instance, PLCore_Renderer* renderer, PLCore_Window* window) {
    vkDeviceWaitIdle(instance.pl_device.device);

    for (uint32_t i = 0; i < renderer->backBuffers; i++) {
        vkDestroyFence(instance.pl_device.device, (*renderer).priv_renderFences[i], VK_NULL_HANDLE);
        vkDestroySemaphore(instance.pl_device.device, (*renderer).priv_waitSemaphores[i], VK_NULL_HANDLE);
        vkDestroySemaphore(instance.pl_device.device, (*renderer).priv_signalSemaphores[i], VK_NULL_HANDLE);
    }

    for (uint32_t i = 0; i < renderer->swapchainImageCount; i++) {
        //vkDestroyImage(instance.pl_device.device, (*renderer).swapchainImages[i], VK_NULL_HANDLE);
        //vkDestroyImageView(instance.pl_device.device, (*renderer).swapchainImageViews[i], VK_NULL_HANDLE);
        vkDestroyFramebuffer(instance.pl_device.device, (*renderer).framebuffers[i], VK_NULL_HANDLE);
    }
    vkDestroyRenderPass(instance.pl_device.device, (*renderer).renderPass, VK_NULL_HANDLE);
    vkDestroyCommandPool(instance.pl_device.device, (*renderer).graphicsPool.pool, VK_NULL_HANDLE);
    vkDestroySwapchainKHR(instance.pl_device.device, (*renderer).swapchain, VK_NULL_HANDLE);
    vkDestroySurfaceKHR(instance.pl_instance.instance, (*window).surface, VK_NULL_HANDLE);

    (*renderer).priv_activeFrame = 0;
}
static void PLCore_CreateRenderObjects(PLCore_RenderInstance instance, PLCore_Renderer* renderer, PLCore_Window* window) {
    int32_t width,height;
    glfwGetWindowSize(window->window, &width, &height);

    if (width <= 0 || height <= 0) printf("Pausing Execution Due To Invalid Window Resolution: %i, %i\n", width, height);
    while (width <= 0 || height <= 0 ) {
        glfwGetWindowSize(window->window, &width, &height);
        glfwWaitEvents();
    }

    PLCore_ResizeWindow(instance, window);

    *renderer = PLCore_CreateRenderer(instance, *window);
}
static void PLCore_RecreateRenderObjects(PLCore_RenderInstance instance, PLCore_Renderer* renderer, PLCore_GraphicsPipeline* pipeline, PLCore_Window* window) {
    PLCore_DestroyRenderObjects(instance, renderer, window);
    PLCore_CreateRenderObjects(instance, renderer, window);
}

void PLCore_RecordCommandBuffer(VkCommandBuffer buffer) {
    VkCommandBufferBeginInfo beginfo;
    beginfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginfo.pNext = VK_NULL_HANDLE;
    beginfo.flags = 0;
    beginfo.pInheritanceInfo = VK_NULL_HANDLE;

    vkBeginCommandBuffer(buffer, &beginfo);
}
void PLCore_StopCommandBuffer(VkCommandBuffer buffer) {
    vkEndCommandBuffer(buffer);
}
void PLCore_SubmitCommandBuffer(VkCommandBuffer buffer, VkQueue queue, VkSemaphore* wait, VkSemaphore* signal, VkFence fence) {
    VkSubmitInfo submitInfo;
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pNext = VK_NULL_HANDLE;
    if (wait != VK_NULL_HANDLE) {
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = wait;
    }
    else {
        submitInfo.waitSemaphoreCount = 0;
        submitInfo.pWaitSemaphores = VK_NULL_HANDLE;
    }

    if (signal != VK_NULL_HANDLE) {
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signal;
    } else {
        submitInfo.signalSemaphoreCount = 0;
        submitInfo.pSignalSemaphores = VK_NULL_HANDLE;
    }

    submitInfo.pWaitDstStageMask = VK_NULL_HANDLE;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &buffer;


    vkQueueSubmit(queue, 1, &submitInfo, fence);
}


PLCore_Buffer PLCore_CreateBuffer(PLCore_RenderInstance instance, VkDeviceSize size, VkBufferUsageFlagBits usage, VkMemoryPropertyFlagBits memoryFlags) {
    VkDeviceMemory memory;
    VkBuffer buffer = PLCore_Priv_CreateBuffer(instance.pl_device.device, instance.pl_device.memoryProperties, size, instance.pl_device.transferQueue.familyIndex, usage, memoryFlags, &memory);
    PLCore_Buffer pl_buffer = {
            .buffer = buffer,
            .memory = memory,
            .bufferInfo = {
                    .buffer = buffer,
                    .offset = 0,
                    .range = size
            }
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
PLCore_Buffer PLCore_CreateUniformBuffer(PLCore_RenderInstance instance, VkDeviceSize size) {
    return PLCore_CreateBuffer(instance, size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, (VkMemoryPropertyFlagBits)(CPU_VISIBLE | CPU_COHERENT));
}
void PLCore_DestroyBuffer(PLCore_RenderInstance instance, PLCore_Buffer* buffer) {
    vkDestroyBuffer(instance.pl_device.device, (*buffer).buffer, VK_NULL_HANDLE);
    vkFreeMemory(instance.pl_device.device, (*buffer).memory, VK_NULL_HANDLE);
}

void PLCore_BeginCommandBuffer(VkCommandBuffer buffer) {
    VkCommandBufferBeginInfo beginInfo;
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.pNext = VK_NULL_HANDLE;
    beginInfo.flags = 0;
    beginInfo.pInheritanceInfo = VK_NULL_HANDLE;
    vkBeginCommandBuffer(buffer, &beginInfo);
}
void PLCore_BeginRenderPass(VkCommandBuffer buffer, VkRenderPass renderPass, VkFramebuffer framebuffer, PLCore_Window* window, PLCore_Renderer* renderer) {
    VkRect2D scissor = window->renderArea;
    VkClearValue clears[2];
    clears[0].color.float32[0] = 0.015f;
    clears[0].color.float32[1] = 0.015f;
    clears[0].color.float32[2] = 0.015f;
    clears[0].color.float32[3] = 1.0f;
    clears[1].depthStencil.depth = 1.0f;
    clears[1].depthStencil.stencil = 0;

    VkRenderPassBeginInfo beginfo;
    beginfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    beginfo.pNext = VK_NULL_HANDLE;
    beginfo.renderPass = renderPass;
    beginfo.framebuffer = framebuffer;
    beginfo.renderArea = scissor;
    beginfo.clearValueCount = 2;
    beginfo.pClearValues = clears;

    vkCmdBeginRenderPass(buffer, &beginfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdSetViewport(renderer->graphicsPool.buffers[renderer->priv_imageIndex], 0, 1, &window->viewport);
    vkCmdSetScissor(renderer->graphicsPool.buffers[renderer->priv_imageIndex], 0, 1, &window->renderArea);
}
uint32_t PLCore_GetImageIndex(PLCore_RenderInstance instance, PLCore_Renderer* renderer) {
    VkResult imageAcquireResult = vkAcquireNextImageKHR(instance.pl_device.device, renderer->swapchain, UINT64_MAX, renderer->priv_waitSemaphores[renderer->priv_activeFrame], VK_NULL_HANDLE, &(*renderer).priv_imageIndex);
    return (*renderer).priv_imageIndex;
}
VkFramebuffer PLCore_GetActiveFrameBuffer(PLCore_Renderer* renderer) {
    return renderer->framebuffers[renderer->priv_imageIndex];
}
void PLCore_BeginFrame(PLCore_RenderInstance instance, PLCore_Renderer* renderer, PLCore_GraphicsPipeline* pipeline, PLCore_Window* window, PLCore_BeginFrameAdditionalInfo* additionalInfo) {
    VkRect2D scissor = window->renderArea;
    VkClearValue clears[2];
    clears[0].color.float32[0] = 0.015f;
    clears[0].color.float32[1] = 0.015f;
    clears[0].color.float32[2] = 0.015f;
    clears[0].color.float32[3] = 1.0f;
    clears[1].depthStencil.depth = 1.0f;
    clears[1].depthStencil.stencil = 0;

    if (additionalInfo == VK_NULL_HANDLE || additionalInfo->beginStage != RENDERPASS_START) {
        VkResult imageAcquireResult = vkAcquireNextImageKHR(instance.pl_device.device, renderer->swapchain, UINT64_MAX, (*renderer).priv_waitSemaphores[renderer->priv_activeFrame], VK_NULL_HANDLE, &(*renderer).priv_imageIndex);
        if (imageAcquireResult == VK_ERROR_OUT_OF_DATE_KHR) {
            printf("Rebuilding Render Objects\n");
            PLCore_RecreateRenderObjects(instance, renderer, pipeline, window);
        }

        vkWaitForFences(instance.pl_device.device, 1, &renderer->priv_renderFences[renderer->priv_activeFrame], VK_TRUE, UINT64_MAX);
        vkResetFences(instance.pl_device.device, 1, &renderer->priv_renderFences[renderer->priv_activeFrame]);

        PLCore_BeginCommandBuffer(renderer->graphicsPool.buffers[renderer->priv_imageIndex]);
    }
    PLCore_BeginRenderPass(renderer->graphicsPool.buffers[renderer->priv_imageIndex], renderer->renderPass, renderer->framebuffers[renderer->priv_imageIndex], window, renderer);
    vkCmdSetViewport(renderer->graphicsPool.buffers[renderer->priv_imageIndex], 0, 1, &window->viewport);
    vkCmdSetScissor(renderer->graphicsPool.buffers[renderer->priv_imageIndex], 0, 1, &window->renderArea);
}
void PLCore_EndFrame(PLCore_RenderInstance instance, PLCore_Renderer* renderer, PLCore_GraphicsPipeline* pipeline, PLCore_Window* window, PLCore_EndFrameAdditionalInfo* additionalInfo) {
    EndFrameRecording(renderer->graphicsPool.buffers[renderer->priv_imageIndex]);

    VkSubmitInfo submitInfo;
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pNext = VK_NULL_HANDLE;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &renderer->graphicsPool.buffers[renderer->priv_imageIndex];

    uint32_t waitCount = 1 + ((additionalInfo == VK_NULL_HANDLE) ? 0 : additionalInfo->additionalSemaphoreCountWait);
    uint32_t signalCount = 1 + ((additionalInfo == VK_NULL_HANDLE) ? 0 : additionalInfo->additionalSemaphoreCountSignal);

    VkSemaphore* waits = malloc(sizeof(VkSemaphore) * waitCount);
    waits[0] = (*renderer).priv_waitSemaphores[renderer->priv_activeFrame];
    if (additionalInfo != VK_NULL_HANDLE)
        // My Dumbass Couldn't Be Worse With Memory, This Took An Hour To Figure Out :(
        // Turns Out The Object Wasn't Even Created Yet
        memcpy(waits+1, additionalInfo->additionalSemaphoresWait, sizeof(VkSemaphore) * additionalInfo->additionalSemaphoreCountWait);

    submitInfo.pWaitSemaphores = waits;
    submitInfo.waitSemaphoreCount = waitCount;


    VkSemaphore* signals = malloc(sizeof(VkSemaphore) * signalCount);
    signals[0] = (*renderer).priv_signalSemaphores[renderer->priv_activeFrame];

    if (additionalInfo != VK_NULL_HANDLE)
        memcpy(signals+1, additionalInfo->additionalSemaphoresSignal, sizeof(VkSemaphore) * additionalInfo->additionalSemaphoreCountSignal);


    submitInfo.pSignalSemaphores = signals;
    submitInfo.signalSemaphoreCount = signalCount;

    VkPipelineStageFlags* waitStages = malloc(sizeof(VkPipelineStageFlags) * waitCount);
    for (int i = 0; i < waitCount; i++)
        waitStages[i] = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;


    submitInfo.pWaitDstStageMask = waitStages;

    vkQueueSubmit(instance.pl_device.graphicsQueue.queue, 1, &submitInfo, (*renderer).priv_renderFences[renderer->priv_activeFrame]);

    VkResult presentationResult;
    VkPresentInfoKHR presentInfo;
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.pNext = VK_NULL_HANDLE;
    presentInfo.waitSemaphoreCount = signalCount;
    presentInfo.pWaitSemaphores = signals;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &renderer->swapchain;
    presentInfo.pImageIndices = &renderer->priv_imageIndex;
    presentInfo.pResults = &presentationResult;

    VkResult renderResult = vkQueuePresentKHR(instance.pl_device.graphicsQueue.queue, &presentInfo);
    if (renderResult == VK_ERROR_OUT_OF_DATE_KHR || renderResult == VK_SUBOPTIMAL_KHR) {
        printf("Out Of Date Render Objects!\n");
        PLCore_RecreateRenderObjects(instance, renderer, pipeline, window);
    }

    (*renderer).priv_activeFrame = (renderer->priv_activeFrame+1)%renderer->backBuffers;
}

VkCommandBuffer PLCore_ActiveRenderBuffer(PLCore_Renderer renderer) {
    return renderer.graphicsPool.buffers[renderer.priv_imageIndex];
}



VkDescriptorSetLayoutBinding
PLCore_Priv_CreateDescriptorLayoutBinding
(uint32_t slot, VkDescriptorType type, uint32_t descriptorCount, VkShaderStageFlagBits stages) {

    VkDescriptorSetLayoutBinding binding;
    binding.binding = slot;
    binding.descriptorType = type;
    binding.descriptorCount = descriptorCount;
    binding.stageFlags = stages;
    binding.pImmutableSamplers = VK_NULL_HANDLE;
    return binding;
}

// TODO: Documentation Of Descriptor Code


/*
VkDescriptorSetLayout PLCore_Priv_CreateDescriptorLayout (VkDevice device, uint32_t bindingCount, VkDescriptorSetLayoutBinding* bindings) {
    VkDescriptorSetLayout layout;
    VkDescriptorSetLayoutCreateInfo layoutInfo;
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.pNext = VK_NULL_HANDLE;
    layoutInfo.flags = 0;
    layoutInfo.bindingCount = bindingCount;
    layoutInfo.pBindings = bindings;

    vkCreateDescriptorSetLayout(device, &layoutInfo, VK_NULL_HANDLE, &layout);
    return layout;
}
VkDescriptorPoolSize PLCore_Priv_CreateDescritorPoolSize (VkDescriptorType type, uint32_t descriptorCount) {
    return (VkDescriptorPoolSize) {
        .type = type,
        .descriptorCount = descriptorCount,
    };
}
VkDescriptorPool PLCore_Priv_CreateDescriptorPool (VkDevice device, uint32_t sets, uint32_t poolSizeCount, VkDescriptorPoolSize* sizes) {
    VkDescriptorPool pool;

    VkDescriptorPoolCreateInfo poolInfo;
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.pNext = VK_NULL_HANDLE;
    poolInfo.flags = 0;
    poolInfo.poolSizeCount = poolSizeCount;
    poolInfo.pPoolSizes = sizes;
    poolInfo.maxSets = sets;

    vkCreateDescriptorPool(device, &poolInfo, VK_NULL_HANDLE, &pool);
    return pool;
}
VkDescriptorSet* PLCore_Priv_CreateDescriptorSets (VkDevice device, uint32_t count, VkDescriptorType type, VkDescriptorSetLayout layout, VkDescriptorPool pool) {
    VkDescriptorSet* sets = malloc(sizeof(VkDescriptorSet) * count);

    VkDescriptorSetLayout* descriptorLayouts = malloc(sizeof(VkDescriptorSetLayout) * count);
    for (uint32_t i = 0; i < count; i++) descriptorLayouts[i] = layout;

    VkDescriptorSetAllocateInfo allocInfo;
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.pNext = VK_NULL_HANDLE;
    allocInfo.pSetLayouts = descriptorLayouts;
    allocInfo.descriptorSetCount = count;
    allocInfo.descriptorPool = pool;

    vkAllocateDescriptorSets(device, &allocInfo, sets);

    free(descriptorLayouts);

    return sets;
}
void PLCore_Priv_WriteDescriptor(VkDevice device, VkDescriptorSet set, VkDescriptorType type, uint32_t dstBinding, VkDescriptorBufferInfo* bufferInfo, VkDescriptorImageInfo* imageInfo) {
    VkWriteDescriptorSet write;
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.pNext = VK_NULL_HANDLE;
    write.dstSet = set;
    write.dstBinding = dstBinding;
    write.dstArrayElement = 0;
    write.descriptorCount = 1;
    write.descriptorType = type;
    write.pImageInfo = imageInfo;
    write.pBufferInfo = bufferInfo;
    write.pTexelBufferView = VK_NULL_HANDLE;

    vkUpdateDescriptorSets(device, 1, &write, 0, VK_NULL_HANDLE);
}


PLCore_DescriptorPool PLCore_CreateDescriptorPoolDetailed(PLCore_RenderInstance instance, uint32_t maxSets, uint32_t sizeCount, VkDescriptorPoolSize* sizes) {
    VkDescriptorPoolCreateInfo info = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .pNext = VK_NULL_HANDLE,
            .flags = 0,
            .maxSets = maxSets,
            .poolSizeCount = sizeCount,
            .pPoolSizes = sizes,
    };
    PLCore_DescriptorPool pool;
    pool.currentAllocations = 0;
    pool.maxAllocations = maxSets;
    vkCreateDescriptorPool(instance.pl_device.device, &info, VK_NULL_HANDLE, &pool.pool);
}
PLCore_DescriptorPool PLCore_CreateDescriptorPool(PLCore_RenderInstance instance, VkDescriptorType typeFlags, uint32_t maxDescriptorAllocations) {
    VkDescriptorPoolSize poolSize = PLCore_Priv_CreateDescritorPoolSize(typeFlags, maxDescriptorAllocations);
    PLCore_DescriptorPool pool = {
            .pool = PLCore_Priv_CreateDescriptorPool(instance.pl_device.device, maxDescriptorAllocations, 1, &poolSize),
            .maxAllocations = maxDescriptorAllocations,
            .currentAllocations = 0,
    };
    return pool;
}
PLCore_Descriptor PLCore_CreateDescriptorFromPool(PLCore_RenderInstance instance, PLCore_DescriptorPool* pool, uint32_t descriptorCount, VkDescriptorType type, uint32_t slot, uint32_t maxBoundAtOnce, VkShaderStageFlagBits stage) {
    PLCore_Descriptor descriptor;
    descriptor.layouts = malloc(sizeof(VkDescriptorSetLayout) * descriptorCount);
    descriptor.sets = malloc(sizeof(VkDescriptorSet) * descriptorCount);
    descriptor.count = descriptorCount;

    for (uint32_t i = 0; i < descriptorCount; i++) {
        VkDescriptorSetLayoutBinding binding = PLCore_Priv_CreateDescriptorLayoutBinding(slot, type, maxBoundAtOnce, stage);
        descriptor.layouts[i] = PLCore_Priv_CreateDescriptorLayout(instance.pl_device.device, 1, &binding);
        descriptor.sets[i] = PLCore_Priv_CreateDescriptorSets(instance.pl_device.device, 1, type, descriptor.layouts[i], pool->pool)[0];
    }
    return descriptor;
}
void PLCore_UpdateDescriptor(PLCore_RenderInstance instance, VkDescriptorSet set, VkDescriptorType type, uint32_t dstBinding, VkDescriptorBufferInfo* bufferInfo, VkDescriptorImageInfo* imageInfo) {
    PLCore_Priv_WriteDescriptor(instance.pl_device.device, set, type, dstBinding, bufferInfo, imageInfo);
}
*/

PLCore_Image PLCore_CreateImage(VkDevice device, VkImageType type, VkFormat format, VkExtent3D extent, VkImageUsageFlagBits usage, uint32_t queueFamilyIndex, VkPhysicalDeviceMemoryProperties memoryProperties) {
    PLCore_Image image;

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
    imageInfo.pQueueFamilyIndices = &queueFamilyIndex;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    vkCreateImage(device, &imageInfo, VK_NULL_HANDLE, &image.image);

    VkMemoryRequirements requirements;
    vkGetImageMemoryRequirements(device, image.image, &requirements);

    VkMemoryAllocateInfo allocInfo;
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.pNext = VK_NULL_HANDLE;
    allocInfo.allocationSize = requirements.size;
    // this means we will be allocating the image to the gpu
    allocInfo.memoryTypeIndex = findMemoryType(memoryProperties, requirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

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
void PLCore_DestroyImage(VkDevice device, PLCore_Image image) {
    vkDestroyImageView(device, image.view, VK_NULL_HANDLE);
    vkDestroyImage(device, image.image, VK_NULL_HANDLE);
    vkFreeMemory(device, image.memory, VK_NULL_HANDLE);
}

void PLCore_TransitionTextureLayout(PLCore_Buffer buffer, PLCore_Image image, uint32_t queueFamily, VkExtent3D extent, VkCommandBuffer commandBuffer, VkFence* waitFence, VkQueue submitQueue) {
    VkCommandBuffer cmd = commandBuffer;

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

    VkSubmitInfo submitInfo = {
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .pNext = VK_NULL_HANDLE,
            .waitSemaphoreCount = 0,
            .pWaitSemaphores = VK_NULL_HANDLE,
            .pWaitDstStageMask = VK_NULL_HANDLE,
            .commandBufferCount = 1,
            .pCommandBuffers = &cmd,
            .signalSemaphoreCount = 0,
            .pSignalSemaphores = VK_NULL_HANDLE,
    };

    vkQueueSubmit(submitQueue, 1, &submitInfo, *waitFence);
}

//#define DEBUG

#ifdef DEBUG

    static size_t DEBUG_time = 0;
    static size_t DEBUG_time_since_last = 0;
    static char DEBUG_title[256] = "";

    #define PLCORE_TIME_TITLED(timerName) \
        strcpy_s(DEBUG_title, 256, timerName); \
        DEBUG_time = clock();             \
        DEBUG_time_since_last = clock();

    #define PLCORE_TIME() \
        printf("Time Taken For %s: %zi\n", DEBUG_title, clock() - DEBUG_time); \
        DEBUG_time = clock();                                                  \
        DEBUG_time_since_last = clock();

    #define PLCORE_CHECK_TIME() \
        printf("\tCurrent Time: %zi\n", clock() - DEBUG_time_since_last); \
        DEBUG_time_since_last = clock();

#else

    #define PLCORE_TIME_TITLED(name)
    #define PLCORE_TIME()
    #define PLCORE_CHECK_TIME()

#endif

PLCore_Texture PLCore_CreateTexture(PLCore_RenderInstance instance, PLCore_Renderer renderer, const char* path) {
    PLCORE_TIME_TITLED("New Texture")
    int32_t width, height, channels;
    void* pixels = stbi_load(path, &width, &height, &channels, STBI_rgb_alpha);
    if (pixels == VK_NULL_HANDLE) fprintf(stderr, "%s", "big sad: loading images\n");

    VkDeviceSize imageSize = width * height * 4;
    VkFormat imageFormat = VK_FORMAT_R8G8B8A8_SRGB;
    VkExtent3D imageExtent = (VkExtent3D){width, height, 1};


    PLCore_Image textureImage = PLCore_CreateImage(instance.pl_device.device, VK_IMAGE_TYPE_2D, imageFormat, imageExtent, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, instance.pl_device.transferQueue.familyIndex, instance.pl_device.memoryProperties);
    PLCORE_CHECK_TIME()

    PLCore_Buffer stagingBuffer = PLCore_CreateBuffer(instance, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, CPU_COHERENT | CPU_VISIBLE);

    PLCore_UploadDataToBuffer(instance.pl_device.device, &stagingBuffer.memory, imageSize, pixels);
    PLCORE_CHECK_TIME()

    stbi_image_free(pixels);

    VkCommandBuffer* cmdBuffer = PLCore_Priv_CreateCommandBuffers(instance.pl_device.device, renderer.graphicsPool.pool, 1);
    PLCORE_CHECK_TIME()

    VkFenceCreateInfo fenceInfo = {
            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
            .pNext = VK_NULL_HANDLE,
            .flags = 0,
    };
    VkFence fence;
    vkCreateFence(instance.pl_device.device, &fenceInfo, VK_NULL_HANDLE, &fence);
    PLCORE_CHECK_TIME()

    PLCore_TransitionTextureLayout(stagingBuffer,
                                   textureImage,
                                   instance.pl_device.graphicsQueue.familyIndex,
                                   imageExtent,
                                   *cmdBuffer,
                                   &fence,
                                   instance.pl_device.graphicsQueue.queue);
    PLCORE_CHECK_TIME()
    vkWaitForFences(instance.pl_device.device, 1, &fence, VK_TRUE, UINT64_MAX);
    vkFreeCommandBuffers(instance.pl_device.device, renderer.graphicsPool.pool, 1, cmdBuffer);
    PLCORE_CHECK_TIME()

    PLCore_Texture texture;
    texture.image = textureImage;

    texture.imageInfo.sampler = VK_NULL_HANDLE;
    texture.imageInfo.imageView = textureImage.view;
    texture.imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    vkDestroyBuffer(instance.pl_device.device, stagingBuffer.buffer, VK_NULL_HANDLE);

    PLCORE_TIME()
    return texture;
}


PLCore_ImageSampler PLCore_CreateSampler(VkDevice device, VkFilter filter, VkSamplerAddressMode addressMode) {
    PLCore_ImageSampler sampler;

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

    vkCreateSampler(device, &samplerInfo, VK_NULL_HANDLE, &sampler.sampler);
    sampler.samplerInfo.sampler = sampler.sampler;
    sampler.samplerInfo.imageView = VK_NULL_HANDLE;
    sampler.samplerInfo.imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    return sampler;
}


PLCore_CameraMoveScheme PLCore_GetDefaultMoveScheme() {
    return (PLCore_CameraMoveScheme){
            .buttonRight = GLFW_KEY_D,
            .buttonLeft = GLFW_KEY_A,
            .buttonForward = GLFW_KEY_W,
            .buttonBackward = GLFW_KEY_S,
            .buttonUp = GLFW_KEY_E,
            .buttonDown = GLFW_KEY_Q,
            .moveSpeedX = 0.25f,
            .moveSpeedY = 0.25f,
            .moveSpeedZ = 0.25f,
            .moveTime = 25,
            .useCameraMouseMovement = 0,
    };
}

PLCore_CameraUniform PLCore_CreateCameraUniform() {
    PLCore_CameraUniform camera = {
        .model = glms_mat4_identity(),
        .view = glms_mat4_identity(),
        .proj = glms_mat4_identity(),
        .moveTimer = clock(),
        .posX = 0.0f,
        .posY = 0.0f,
        .posZ = 0.0f,
        .rotX = 0.0f,
        .rotY = 0.0f,
        .fov = 45,
    };
    return camera;
}
void PLCore_PollCameraMovements(PLCore_Window window, PLCore_CameraUniform* camera, PLCore_CameraMoveScheme scheme) {

    if (scheme.useCameraMouseMovement) {
        double mousePos[2];
        glfwGetCursorPos(window.window, &mousePos[0], &mousePos[1]);
        mousePos[0] /= (float) window.resolution.width;
        mousePos[1] /= (float) window.resolution.height;

        (*camera).rotX = (((float) -mousePos[0] + 0.5f) * ((float) window.resolution.width * 0.001f)) * 1.5f;
        (*camera).rotY = (((float) -mousePos[1] + 0.5f) * ((float) window.resolution.height * 0.001f)) * 1.5f;
    }
    if (clock() - camera->moveTimer > scheme.moveTime) {
        if (glfwGetKey(window.window, scheme.buttonLeft)) { (*camera).posX += scheme.moveSpeedX; }
        if (glfwGetKey(window.window, scheme.buttonRight)) { (*camera).posX -= scheme.moveSpeedX; }
        if (glfwGetKey(window.window, scheme.buttonUp)) { (*camera).posY += scheme.moveSpeedY; }
        if (glfwGetKey(window.window, scheme.buttonDown)) { (*camera).posY -= scheme.moveSpeedY; }
        if (glfwGetKey(window.window, scheme.buttonForward)) { (*camera).posZ += scheme.moveSpeedZ; }
        if (glfwGetKey(window.window, scheme.buttonBackward)) { (*camera).posZ -= scheme.moveSpeedZ; }
        (*camera).moveTimer = clock();
    }

    (*camera).model = glms_translate(glms_mat4_identity(), (vec3s){camera->posX * scheme.moveSpeedX, camera->posY * scheme.moveSpeedY, camera->posZ * scheme.moveSpeedZ});
    (*camera).view = glms_lookat((vec3s){
                                           camera->rotX * 1.0f,
                                           camera->rotY * 1.0f,
                                           2.0f,
                                   },
                                   (vec3s){0.0f, 0.0f, 0.0f},(vec3s){0.0f,1.0f,0.0f});
    (*camera).proj = glms_perspective(glm_rad(camera->fov), (float)window.resolution.width / (float)window.resolution.height, 0.1f, 10.0f);
}




// `types` Needs To Be An Array
// `descriptorSetCount` Needs To Be An Array
// `count` Specifies How Large The `descriptorSetCount` and `types` Array Are: `descriptorSetCount` and `types` Should Be The Same Size

_CRT_DEPRECATE_TEXT("Prefer Other Function")
PLCore_DescriptorPoolAllocator PLCore_CreateDescriptorPoolAllocator(uint32_t descriptorSlot, VkDescriptorType* types, uint32_t* descriptorSetCount, uint32_t count, uint32_t maxDescriptorSets, VkShaderStageFlagBits shaderStage) {

    static const uint32_t DESCRIPTOR_ALLOC_COUNT_PADDING = 5;
    VkDescriptorPoolSize* sizes = (VkDescriptorPoolSize*)malloc(sizeof(VkDescriptorPoolSize) * count);
    VkDescriptorSetLayoutBinding* bindings = (VkDescriptorSetLayoutBinding*)malloc(sizeof(VkDescriptorSetLayoutBinding) * count);

    for (int i = 0; i < count; i++) {
        sizes[i].descriptorCount = (descriptorSetCount[i] + DESCRIPTOR_ALLOC_COUNT_PADDING);
        sizes[i].type = types[i];

        bindings[i].binding = descriptorSlot;
        bindings[i].descriptorType = types[i];
        bindings[i].descriptorCount = descriptorSetCount[i];
        bindings[i].stageFlags = (VkShaderStageFlags)shaderStage;
        bindings[i].pImmutableSamplers = VK_NULL_HANDLE;
    }

    PLCore_DescriptorPoolAllocator poolAllocator;
    poolAllocator.sizes = sizes;
    poolAllocator.poolSizeCount = count;
    poolAllocator.bindings = bindings;
    poolAllocator.bindingCount = count;
    poolAllocator.types = types;
    poolAllocator.typeCount = count;
    poolAllocator.maxDescriptorSets = maxDescriptorSets;

    return poolAllocator;
}


_CRT_DEPRECATE_TEXT("Prefer Other Function")
PLCore_DescriptorPoolAllocator PLCore_CreateDescriptorPoolFromAllocator(VkDevice device, PLCore_DescriptorPoolAllocator allocator) {
    VkDescriptorPoolCreateInfo info = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .pNext = VK_NULL_HANDLE,
            .flags = 0,
            .maxSets = allocator.maxDescriptorSets,
            .poolSizeCount = allocator.poolSizeCount,
            .pPoolSizes = allocator.sizes,
    };
    vkCreateDescriptorPool(device, &info, VK_NULL_HANDLE, &allocator.pool);
    // This Returns A Local Copy
    // It Does This So You Can Chain Functions Together
    // Problems:
    //      If You Expect The Function To Return Nothing The Allocator Will Be Invalid For Creaing Descriptor Sets
    return allocator;
}


_CRT_DEPRECATE_TEXT("Prefer Other Function")
PLCore_DescriptorSet PLCore_CreateDescriptorSets(VkDevice device, VkDescriptorType typeFlags, PLCore_DescriptorPoolAllocator allocator) {

    // Setting This To 20 Because I Dont Really Know What Else To Set It To
    // Binding Counts Should Really Never Be Over 3 (In My Mind)
    const uint32_t MAX_BINDINGS = 20;

    uint32_t bindingCount = 0;
    VkDescriptorSetLayoutBinding* bindings = (VkDescriptorSetLayoutBinding*)malloc(sizeof(VkDescriptorSetLayoutBinding) * MAX_BINDINGS);


    // Search Through The Allocators Bindings To Find The Correct Descriptor Type
    for (int i = 0; i < allocator.bindingCount; i++) {
        if (allocator.types[i] & typeFlags) {
            bindings[i] = allocator.bindings[i];
            bindingCount++;
        }
    }


    VkDescriptorSetLayoutCreateInfo layoutInfo = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .pNext = VK_NULL_HANDLE,
            .flags = 0,
            .bindingCount = bindingCount,
            .pBindings = bindings,
    };
    VkDescriptorSetLayout layout;
    vkCreateDescriptorSetLayout(device, &layoutInfo, VK_NULL_HANDLE, &layout);

    VkDescriptorSetAllocateInfo allocInfo;
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.pNext = VK_NULL_HANDLE;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &layout;
    allocInfo.descriptorPool = allocator.pool;

    VkDescriptorSet set;
    vkAllocateDescriptorSets(device, &allocInfo, &set);

    PLCore_DescriptorSet descriptorSet;
    descriptorSet.set = set;
    descriptorSet.layout = layout;

    return descriptorSet;
}

void PLCore_UpdateDescriptor(
        PLCore_RenderInstance instance,
        VkDescriptorSet set,
        VkDescriptorType type,
        uint32_t dstBinding,
        VkDescriptorBufferInfo* bufferInfo,
        VkDescriptorImageInfo* imageInfo,
        PLCore_DescriptorAdditionalInfo* additionalInfo)
{
    VkWriteDescriptorSet write;
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.pNext = VK_NULL_HANDLE;
    write.dstSet = set;
    write.dstBinding = dstBinding;
    write.dstArrayElement = (additionalInfo == VK_NULL_HANDLE) ? 0 : additionalInfo->arrayOffset;
    write.descriptorCount = (additionalInfo == VK_NULL_HANDLE) ? 1 : additionalInfo->arrayCount;
    write.descriptorType = type;
    write.pImageInfo = imageInfo;
    write.pBufferInfo = bufferInfo;
    write.pTexelBufferView = VK_NULL_HANDLE;

    printf("Updating Set With Type: %i\n", (int)type);
    vkUpdateDescriptorSets(instance.pl_device.device, 1, &write, 0, VK_NULL_HANDLE);
}

/*
PLCore_ReflectedDescriptorSet scanShaders(PLCore_RenderInstance instance, PLCore_ShaderModule module) {
    PLCore_ReflectedDescriptorSet reflectedSets;


    SpvReflectShaderModule mod;
    spvReflectCreateShaderModule(module.size, module.buffer, &mod);

    uint32_t descriptorCount = 0;
    spvReflectEnumerateDescriptorSets(&mod, &descriptorCount, VK_NULL_HANDLE);
    SpvReflectDescriptorSet** descriptorSets = malloc(sizeof(SpvReflectDescriptorSet*) * descriptorCount);
    spvReflectEnumerateDescriptorSets(&mod, &descriptorCount, descriptorSets);
    reflectedSets.descriptorSetCount = malloc(sizeof(uint32_t) * descriptorCount);
    reflectedSets.sets = malloc(sizeof(VkDescriptorSet) * descriptorCount);

    const uint32_t DESCRIPTORS_PER_TYPE = 25;
    VkDescriptorPoolSize sizes[11] = {
            {VK_DESCRIPTOR_TYPE_SAMPLER, DESCRIPTORS_PER_TYPE},
            {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, DESCRIPTORS_PER_TYPE},
            {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, DESCRIPTORS_PER_TYPE},
            {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, DESCRIPTORS_PER_TYPE},
            {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, DESCRIPTORS_PER_TYPE},
            {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, DESCRIPTORS_PER_TYPE},
            {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, DESCRIPTORS_PER_TYPE},
            {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, DESCRIPTORS_PER_TYPE},
            {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, DESCRIPTORS_PER_TYPE},
            {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, DESCRIPTORS_PER_TYPE},
            {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, DESCRIPTORS_PER_TYPE},
    };
    VkDescriptorPoolCreateInfo poolInfo;
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.pNext = VK_NULL_HANDLE;
    poolInfo.flags = 0;
    poolInfo.poolSizeCount = 11;
    poolInfo.pPoolSizes = sizes;
    // Descriptor Types * DESCRIPTORS_PER_TYPE
    poolInfo.maxSets = 11 * DESCRIPTORS_PER_TYPE;

    VkDescriptorPool pool;
    PLCORE_RESULT("Descriptor Pool Creation", vkCreateDescriptorPool(instance.pl_device.device, &poolInfo, VK_NULL_HANDLE, &pool));
    reflectedSets.pool = pool;

    for (int set = 0; set < descriptorCount; set++) {
        printf("Set: %u/%u\n", descriptorSets[set]->set+1, descriptorCount);
        reflectedSets.sets[set].bindings = malloc(sizeof(VkDescriptorSetLayoutBinding) * descriptorSets[set]->binding_count);
        reflectedSets.sets[set].bindingCount = descriptorSets[set]->binding_count;
        for (int binding = 0; binding < descriptorSets[set]->binding_count; binding++) {

            printf("\tBinding: \"%s\"\n", descriptorSets[set]->bindings[binding]->name);
            printf("\tType: %u\n", descriptorSets[set]->bindings[binding]->descriptor_type);

            reflectedSets.sets[set].bindings[binding].binding = descriptorSets[set]->bindings[binding]->binding;
            reflectedSets.sets[set].bindings[binding].descriptorCount = descriptorSets[set]->bindings[binding]->count;
            reflectedSets.sets[set].bindings[binding].descriptorType = (VkDescriptorType)descriptorSets[set]->bindings[binding]->descriptor_type;
            reflectedSets.sets[set].bindings[binding].pImmutableSamplers = VK_NULL_HANDLE;
            reflectedSets.sets[set].bindings[binding].stageFlags = module.stage;


        }
        printf("\tBinding Count: %u\n", descriptorSets[set]->binding_count);

        VkDescriptorSetLayout layout;
        VkDescriptorSetLayoutCreateInfo layoutInfo;
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.pNext = VK_NULL_HANDLE;
        layoutInfo.flags = 0;
        layoutInfo.bindingCount = reflectedSets.sets[0].bindingCount;
        layoutInfo.pBindings = reflectedSets.sets[0].bindings;
        //PLCORE_RESULT("Descriptor Layout Creation", vkCreateDescriptorSetLayout(instance.pl_device.device, &layoutInfo, VK_NULL_HANDLE, &layout));
        vkCreateDescriptorSetLayout(instance.pl_device.device, &layoutInfo, VK_NULL_HANDLE, &layout);


        VkDescriptorSetAllocateInfo allocInfo;
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.pNext = VK_NULL_HANDLE;
        allocInfo.descriptorPool = reflectedSets.pool;
        allocInfo.descriptorSetCount = 1;
        allocInfo.pSetLayouts = &layout;

        VkDescriptorSet descriptorSet;
        //PLCORE_RESULT("Descriptor Allocation Result", vkAllocateDescriptorSets(instance.pl_device.device, &allocInfo, &descriptorSet));
        vkAllocateDescriptorSets(instance.pl_device.device, &allocInfo, &descriptorSet);

        reflectedSets.descriptorSetCount[set] = 1;
        reflectedSets.sets[set].set = descriptorSet;
        reflectedSets.sets[set].layout = layout;
        reflectedSets.sets[set].slot = descriptorSets[set]->set;

        printf("Check Set Setter\n");

        VkWriteDescriptorSet write;
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.pNext = VK_NULL_HANDLE;
        write.dstSet = reflectedSets.sets[set].set;
        write.dstBinding = 0;
        write.dstArrayElement = 0;
        write.descriptorCount = 1;
        write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        write.pImageInfo = VK_NULL_HANDLE;
        write.pBufferInfo = VK_NULL_HANDLE;
        write.pTexelBufferView = VK_NULL_HANDLE;

        reflectedSets.sets[set].write = write;

        printf("Check Write Setter\n");
    }
    return reflectedSets;
}

static PLCore_EXP_shaderDescriptors PLCore_EXP_newShaderDescriptor(uint32_t descriptorCount) {
    PLCore_EXP_shaderDescriptors descriptor;
    descriptor.descriptorCount = malloc(sizeof(uint32_t) * descriptorCount);
    descriptor.sets = malloc(sizeof(VkDescriptorSet**) * descriptorCount);
    descriptor.layouts = malloc(sizeof(VkDescriptorSetLayout**) * descriptorCount);
    descriptor.write = malloc(sizeof(VkWriteDescriptorSet) * descriptorCount);
    return descriptor;
}

PLCore_EXP_shaderDescriptors PLCore_EXP_ReflectShader(PLCore_RenderInstance instance, PLCore_ShaderModule module) {



    SpvReflectShaderModule mod;
    spvReflectCreateShaderModule(module.size, module.buffer, &mod);

    uint32_t descriptorCount = 0;
    spvReflectEnumerateDescriptorSets(&mod, &descriptorCount, VK_NULL_HANDLE);
    SpvReflectDescriptorSet** descriptorSets = malloc(sizeof(SpvReflectDescriptorSet*) * descriptorCount);
    spvReflectEnumerateDescriptorSets(&mod, &descriptorCount, descriptorSets);

    const uint32_t DESCRIPTORS_PER_TYPE = 25;
    VkDescriptorPoolSize sizes[11] = {
            {VK_DESCRIPTOR_TYPE_SAMPLER, DESCRIPTORS_PER_TYPE},
            {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, DESCRIPTORS_PER_TYPE},
            {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, DESCRIPTORS_PER_TYPE},
            {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, DESCRIPTORS_PER_TYPE},
            {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, DESCRIPTORS_PER_TYPE},
            {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, DESCRIPTORS_PER_TYPE},
            {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, DESCRIPTORS_PER_TYPE},
            {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, DESCRIPTORS_PER_TYPE},
            {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, DESCRIPTORS_PER_TYPE},
            {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, DESCRIPTORS_PER_TYPE},
            {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, DESCRIPTORS_PER_TYPE},
    };
    VkDescriptorPoolCreateInfo poolInfo;
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.pNext = VK_NULL_HANDLE;
    poolInfo.flags = 0;
    poolInfo.poolSizeCount = 11;
    poolInfo.pPoolSizes = sizes;
    // Descriptor Types * DESCRIPTORS_PER_TYPE
    poolInfo.maxSets = 11 * DESCRIPTORS_PER_TYPE;

    VkDescriptorPool pool;
    PLCORE_RESULT("Descriptor Pool Creation", vkCreateDescriptorPool(instance.pl_device.device, &poolInfo, VK_NULL_HANDLE, &pool));

    PLCore_EXP_shaderDescriptors descriptor =  PLCore_EXP_newShaderDescriptor(descriptorCount);

    for (int set = 0; set < descriptorCount; set++) {
        VkDescriptorSetLayoutBinding* bindings = malloc(sizeof(VkDescriptorSetLayoutBinding) * descriptorSets[set]->binding_count);
        for (int binding = 0; binding < descriptorSets[set]->binding_count; binding++) {
            bindings[binding].binding = descriptorSets[set]->bindings[binding]->binding;
            bindings[binding].descriptorCount = descriptorSets[set]->bindings[binding]->count;
            bindings[binding].descriptorType = (VkDescriptorType)descriptorSets[set]->bindings[binding]->descriptor_type;
            bindings[binding].pImmutableSamplers = VK_NULL_HANDLE;
            bindings[binding].stageFlags = module.stage;
        }


        VkDescriptorSetLayoutCreateInfo layoutInfo;
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.pNext = VK_NULL_HANDLE;
        layoutInfo.flags = 0;
        layoutInfo.bindingCount = descriptorSets[set]->binding_count;
        layoutInfo.pBindings = bindings;
        //PLCORE_RESULT("Descriptor Layout Creation", vkCreateDescriptorSetLayout(instance.pl_device.device, &layoutInfo, VK_NULL_HANDLE, &layout));
        vkCreateDescriptorSetLayout(instance.pl_device.device, &layoutInfo, VK_NULL_HANDLE, &(descriptor.layouts[set][0]));


        VkDescriptorSetAllocateInfo allocInfo;
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.pNext = VK_NULL_HANDLE;
        allocInfo.descriptorPool = pool;
        allocInfo.descriptorSetCount = 1;
        allocInfo.pSetLayouts = &descriptor.layouts[set][0];

        VkDescriptorSet descriptorSet;
        //PLCORE_RESULT("Descriptor Allocation Result", vkAllocateDescriptorSets(instance.pl_device.device, &allocInfo, &descriptorSet));
        vkAllocateDescriptorSets(instance.pl_device.device, &allocInfo, &descriptorSet);

        descriptor.sets[set][0] = descriptorSet;
        descriptor.descriptorCount[set] = 1;

        printf("Check Set Setter\n");

        VkWriteDescriptorSet write;
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.pNext = VK_NULL_HANDLE;
        write.dstSet = descriptor.sets[set][0];
        write.dstBinding = 0;
        write.dstArrayElement = 0;
        write.descriptorCount = 1;
        write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        write.pImageInfo = VK_NULL_HANDLE;
        write.pBufferInfo = VK_NULL_HANDLE;
        write.pTexelBufferView = VK_NULL_HANDLE;

        descriptor.write[set] = write;

        printf("Check Write Setter\n");
    }
    return descriptor;
}
*/

//static const uint32_t
#define DESCRIPTORS_PER_TYPE 25
//static const uint32_t
#define DESCRIPTORS_TYPE_COUNT 11
VkDescriptorPool PLCore_CreateGeneralizedDescriptorPool(PLCore_RenderInstance instance) {
    VkDescriptorPoolSize sizes[DESCRIPTORS_TYPE_COUNT] = {
            {VK_DESCRIPTOR_TYPE_SAMPLER, DESCRIPTORS_PER_TYPE},
            {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, DESCRIPTORS_PER_TYPE},
            {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, DESCRIPTORS_PER_TYPE},
            {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, DESCRIPTORS_PER_TYPE},
            {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, DESCRIPTORS_PER_TYPE},
            {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, DESCRIPTORS_PER_TYPE},
            {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, DESCRIPTORS_PER_TYPE},
            {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, DESCRIPTORS_PER_TYPE},
            {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, DESCRIPTORS_PER_TYPE},
            {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, DESCRIPTORS_PER_TYPE},
            {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, DESCRIPTORS_PER_TYPE},
    };
    VkDescriptorPoolCreateInfo poolInfo;
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.pNext = VK_NULL_HANDLE;
    poolInfo.flags = 0;
    poolInfo.poolSizeCount = 11;
    poolInfo.pPoolSizes = sizes;
    poolInfo.maxSets = DESCRIPTORS_TYPE_COUNT * DESCRIPTORS_PER_TYPE;

    VkDescriptorPool pool;
    vkCreateDescriptorPool(instance.pl_device.device, &poolInfo, VK_NULL_HANDLE, &pool);
    return pool;
}

VkDescriptorSetLayoutBinding PLCore_CreateDescriptorSetLayoutBinding(uint32_t slot, uint32_t count, VkDescriptorType type, VkShaderStageFlags stage) {
    VkDescriptorSetLayoutBinding binding;
    binding.binding = slot;
    binding.descriptorType = type;
    binding.descriptorCount = count;
    binding.stageFlags = stage;
    binding.pImmutableSamplers = VK_NULL_HANDLE;
    return binding;
}

VkDescriptorSet PLCore_CreateDescriptorSetAdvanced(PLCore_RenderInstance instance, VkDescriptorPool pool, uint32_t bindingCount, VkDescriptorSetLayoutBinding* bindings, VkShaderStageFlags stage, VkDescriptorSetLayout* layout) {
    VkDescriptorSetLayoutCreateInfo layoutInfo;
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.pNext = VK_NULL_HANDLE;
    layoutInfo.flags = 0;
    layoutInfo.bindingCount = bindingCount;
    layoutInfo.pBindings = bindings;

    if (layout == VK_NULL_HANDLE)
        assert(1);

    vkCreateDescriptorSetLayout(instance.pl_device.device, &layoutInfo, VK_NULL_HANDLE, layout);


    VkDescriptorSetAllocateInfo allocInfo;
    allocInfo.pNext = VK_NULL_HANDLE;
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = layout;
    allocInfo.descriptorPool = pool;

    VkDescriptorSet set;
    vkAllocateDescriptorSets(instance.pl_device.device, &allocInfo, &set);

    return set;
}

VkDescriptorSet PLCore_CreateDescriptorSet(PLCore_RenderInstance instance, VkDescriptorPool pool, uint32_t slot, VkDescriptorType type, VkShaderStageFlags stage, VkDescriptorSetLayout* layout) {

    VkDescriptorSetLayoutBinding binding;
    binding.binding = slot;
    binding.descriptorType = type;
    binding.descriptorCount = 1;
    binding.stageFlags = stage;
    binding.pImmutableSamplers = VK_NULL_HANDLE;

    VkDescriptorSetLayoutCreateInfo layoutInfo;
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.pNext = VK_NULL_HANDLE;
    layoutInfo.flags = 0;
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &binding;
    if (layout == VK_NULL_HANDLE)
        assert(1);

    vkCreateDescriptorSetLayout(instance.pl_device.device, &layoutInfo, VK_NULL_HANDLE, layout);


    VkDescriptorSetAllocateInfo allocInfo;
    allocInfo.pNext = VK_NULL_HANDLE;
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = layout;
    allocInfo.descriptorPool = pool;

    VkDescriptorSet set;
    vkAllocateDescriptorSets(instance.pl_device.device, &allocInfo, &set);

    return set;
}

PLCore_DescriptorSet* PLCore_CreateDescriptorSetFromShader(PLCore_RenderInstance instance, PLCore_ShaderModule shader, VkDescriptorPool allcationPool, uint32_t* out_DescriptorCount) {

    SpvReflectShaderModule mod;
    spvReflectCreateShaderModule(shader.size, shader.buffer, &mod);

    uint32_t descriptorCount = 0;
    spvReflectEnumerateDescriptorSets(&mod, &descriptorCount, VK_NULL_HANDLE);
    SpvReflectDescriptorSet** descriptorInfos = malloc(sizeof(SpvReflectDescriptorSet*) * descriptorCount);
    spvReflectEnumerateDescriptorSets(&mod, &descriptorCount, descriptorInfos);

    PLCore_DescriptorSet* sets = malloc(sizeof(PLCore_DescriptorSet) * descriptorCount);
    if (out_DescriptorCount != VK_NULL_HANDLE)
        *out_DescriptorCount = descriptorCount;

    for (int i = 0; i < descriptorCount; i++) {
        sets[i].slot = descriptorInfos[i]->set;
        sets[i].bindings = malloc(sizeof(PLCore_DescriptorBinding) * descriptorInfos[i]->binding_count);
        sets[i].bindingCount = descriptorInfos[i]->binding_count;
        //descriptorInfos[i]; this is the set
        VkDescriptorSetLayoutBinding* bindings = malloc(sizeof(VkDescriptorSetLayoutBinding) * descriptorInfos[i]->binding_count);
        for (int j = 0; j < descriptorInfos[i]->binding_count; j++) {
            sets[i].bindings[j].arrayCount =
                    bindings[j].descriptorCount = descriptorInfos[i]->bindings[j]->count;
            sets[i].bindings[j].bindingSlot =
                    bindings[j].binding = descriptorInfos[i]->bindings[j]->binding;
            sets[i].bindings[j].type =
                    bindings[j].descriptorType = (VkDescriptorType)descriptorInfos[i]->bindings[j]->descriptor_type;
            sets[i].bindings[j].name =
                    descriptorInfos[i]->bindings[j]->name;
            bindings[j].pImmutableSamplers = VK_NULL_HANDLE;
            bindings[j].stageFlags = shader.stage;

            //printf("Shader Descriptor: [\"%s\"]:\n\t Set: %i  |  Count: %i\n", descriptorInfos[i]->bindings[j]->name, descriptorInfos[i]->bindings[j]->set, descriptorInfos[i]->bindings[j]->count);

            sets[i].bindings[j].write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            sets[i].bindings[j].write.pNext = VK_NULL_HANDLE;
            // The Set To Update Does Not Currently Exist So We Need To Save `dstSet` Member For Later
            sets[i].bindings[j].write.dstBinding = bindings[j].binding;
            sets[i].bindings[j].write.dstArrayElement = 0;
            sets[i].bindings[j].write.descriptorCount = bindings[j].descriptorCount;
            sets[i].bindings[j].write.descriptorType = bindings[j].descriptorType;
            sets[i].bindings[j].write.pImageInfo = VK_NULL_HANDLE;
            sets[i].bindings[j].write.pBufferInfo = VK_NULL_HANDLE;
            sets[i].bindings[j].write.pTexelBufferView = VK_NULL_HANDLE;
        }
        sets[i].set = PLCore_CreateDescriptorSetAdvanced(instance, allcationPool, descriptorInfos[i]->binding_count, bindings, shader.stage, &sets[i].layout);
        for (int j  = 0; j < descriptorInfos[i]->binding_count; j++)
            sets[i].bindings[j].write.dstSet = sets[i].set;
    }
    for (int i = 0; i < descriptorCount; i++) {
        printf("Creating Descriptors From Shader '%s'  |  Binding Count: %i\n", shader.path, sets[i].bindingCount);
        for (int j = 0; j < sets[i].bindingCount; j++) {
            printf("\tName: '%s'\n\t\tArray Count: %i\n",
                   sets[i].bindings[j].name,
                   sets[i].bindings[j].arrayCount);
        }
    }

    return sets;
}


static bool compareAttributeDesc(const VkVertexInputAttributeDescription* a, const VkVertexInputAttributeDescription* b) {
    return a->location > b->location;
}

static uint32_t FormatSize(VkFormat format)
{
    uint32_t result = 0;
    switch (format) {
        case VK_FORMAT_UNDEFINED: result = 0; break;
        case VK_FORMAT_R4G4_UNORM_PACK8: result = 1; break;
        case VK_FORMAT_R4G4B4A4_UNORM_PACK16: result = 2; break;
        case VK_FORMAT_B4G4R4A4_UNORM_PACK16: result = 2; break;
        case VK_FORMAT_R5G6B5_UNORM_PACK16: result = 2; break;
        case VK_FORMAT_B5G6R5_UNORM_PACK16: result = 2; break;
        case VK_FORMAT_R5G5B5A1_UNORM_PACK16: result = 2; break;
        case VK_FORMAT_B5G5R5A1_UNORM_PACK16: result = 2; break;
        case VK_FORMAT_A1R5G5B5_UNORM_PACK16: result = 2; break;
        case VK_FORMAT_R8_UNORM: result = 1; break;
        case VK_FORMAT_R8_SNORM: result = 1; break;
        case VK_FORMAT_R8_USCALED: result = 1; break;
        case VK_FORMAT_R8_SSCALED: result = 1; break;
        case VK_FORMAT_R8_UINT: result = 1; break;
        case VK_FORMAT_R8_SINT: result = 1; break;
        case VK_FORMAT_R8_SRGB: result = 1; break;
        case VK_FORMAT_R8G8_UNORM: result = 2; break;
        case VK_FORMAT_R8G8_SNORM: result = 2; break;
        case VK_FORMAT_R8G8_USCALED: result = 2; break;
        case VK_FORMAT_R8G8_SSCALED: result = 2; break;
        case VK_FORMAT_R8G8_UINT: result = 2; break;
        case VK_FORMAT_R8G8_SINT: result = 2; break;
        case VK_FORMAT_R8G8_SRGB: result = 2; break;
        case VK_FORMAT_R8G8B8_UNORM: result = 3; break;
        case VK_FORMAT_R8G8B8_SNORM: result = 3; break;
        case VK_FORMAT_R8G8B8_USCALED: result = 3; break;
        case VK_FORMAT_R8G8B8_SSCALED: result = 3; break;
        case VK_FORMAT_R8G8B8_UINT: result = 3; break;
        case VK_FORMAT_R8G8B8_SINT: result = 3; break;
        case VK_FORMAT_R8G8B8_SRGB: result = 3; break;
        case VK_FORMAT_B8G8R8_UNORM: result = 3; break;
        case VK_FORMAT_B8G8R8_SNORM: result = 3; break;
        case VK_FORMAT_B8G8R8_USCALED: result = 3; break;
        case VK_FORMAT_B8G8R8_SSCALED: result = 3; break;
        case VK_FORMAT_B8G8R8_UINT: result = 3; break;
        case VK_FORMAT_B8G8R8_SINT: result = 3; break;
        case VK_FORMAT_B8G8R8_SRGB: result = 3; break;
        case VK_FORMAT_R8G8B8A8_UNORM: result = 4; break;
        case VK_FORMAT_R8G8B8A8_SNORM: result = 4; break;
        case VK_FORMAT_R8G8B8A8_USCALED: result = 4; break;
        case VK_FORMAT_R8G8B8A8_SSCALED: result = 4; break;
        case VK_FORMAT_R8G8B8A8_UINT: result = 4; break;
        case VK_FORMAT_R8G8B8A8_SINT: result = 4; break;
        case VK_FORMAT_R8G8B8A8_SRGB: result = 4; break;
        case VK_FORMAT_B8G8R8A8_UNORM: result = 4; break;
        case VK_FORMAT_B8G8R8A8_SNORM: result = 4; break;
        case VK_FORMAT_B8G8R8A8_USCALED: result = 4; break;
        case VK_FORMAT_B8G8R8A8_SSCALED: result = 4; break;
        case VK_FORMAT_B8G8R8A8_UINT: result = 4; break;
        case VK_FORMAT_B8G8R8A8_SINT: result = 4; break;
        case VK_FORMAT_B8G8R8A8_SRGB: result = 4; break;
        case VK_FORMAT_A8B8G8R8_UNORM_PACK32: result = 4; break;
        case VK_FORMAT_A8B8G8R8_SNORM_PACK32: result = 4; break;
        case VK_FORMAT_A8B8G8R8_USCALED_PACK32: result = 4; break;
        case VK_FORMAT_A8B8G8R8_SSCALED_PACK32: result = 4; break;
        case VK_FORMAT_A8B8G8R8_UINT_PACK32: result = 4; break;
        case VK_FORMAT_A8B8G8R8_SINT_PACK32: result = 4; break;
        case VK_FORMAT_A8B8G8R8_SRGB_PACK32: result = 4; break;
        case VK_FORMAT_A2R10G10B10_UNORM_PACK32: result = 4; break;
        case VK_FORMAT_A2R10G10B10_SNORM_PACK32: result = 4; break;
        case VK_FORMAT_A2R10G10B10_USCALED_PACK32: result = 4; break;
        case VK_FORMAT_A2R10G10B10_SSCALED_PACK32: result = 4; break;
        case VK_FORMAT_A2R10G10B10_UINT_PACK32: result = 4; break;
        case VK_FORMAT_A2R10G10B10_SINT_PACK32: result = 4; break;
        case VK_FORMAT_A2B10G10R10_UNORM_PACK32: result = 4; break;
        case VK_FORMAT_A2B10G10R10_SNORM_PACK32: result = 4; break;
        case VK_FORMAT_A2B10G10R10_USCALED_PACK32: result = 4; break;
        case VK_FORMAT_A2B10G10R10_SSCALED_PACK32: result = 4; break;
        case VK_FORMAT_A2B10G10R10_UINT_PACK32: result = 4; break;
        case VK_FORMAT_A2B10G10R10_SINT_PACK32: result = 4; break;
        case VK_FORMAT_R16_UNORM: result = 2; break;
        case VK_FORMAT_R16_SNORM: result = 2; break;
        case VK_FORMAT_R16_USCALED: result = 2; break;
        case VK_FORMAT_R16_SSCALED: result = 2; break;
        case VK_FORMAT_R16_UINT: result = 2; break;
        case VK_FORMAT_R16_SINT: result = 2; break;
        case VK_FORMAT_R16_SFLOAT: result = 2; break;
        case VK_FORMAT_R16G16_UNORM: result = 4; break;
        case VK_FORMAT_R16G16_SNORM: result = 4; break;
        case VK_FORMAT_R16G16_USCALED: result = 4; break;
        case VK_FORMAT_R16G16_SSCALED: result = 4; break;
        case VK_FORMAT_R16G16_UINT: result = 4; break;
        case VK_FORMAT_R16G16_SINT: result = 4; break;
        case VK_FORMAT_R16G16_SFLOAT: result = 4; break;
        case VK_FORMAT_R16G16B16_UNORM: result = 6; break;
        case VK_FORMAT_R16G16B16_SNORM: result = 6; break;
        case VK_FORMAT_R16G16B16_USCALED: result = 6; break;
        case VK_FORMAT_R16G16B16_SSCALED: result = 6; break;
        case VK_FORMAT_R16G16B16_UINT: result = 6; break;
        case VK_FORMAT_R16G16B16_SINT: result = 6; break;
        case VK_FORMAT_R16G16B16_SFLOAT: result = 6; break;
        case VK_FORMAT_R16G16B16A16_UNORM: result = 8; break;
        case VK_FORMAT_R16G16B16A16_SNORM: result = 8; break;
        case VK_FORMAT_R16G16B16A16_USCALED: result = 8; break;
        case VK_FORMAT_R16G16B16A16_SSCALED: result = 8; break;
        case VK_FORMAT_R16G16B16A16_UINT: result = 8; break;
        case VK_FORMAT_R16G16B16A16_SINT: result = 8; break;
        case VK_FORMAT_R16G16B16A16_SFLOAT: result = 8; break;
        case VK_FORMAT_R32_UINT: result = 4; break;
        case VK_FORMAT_R32_SINT: result = 4; break;
        case VK_FORMAT_R32_SFLOAT: result = 4; break;
        case VK_FORMAT_R32G32_UINT: result = 8; break;
        case VK_FORMAT_R32G32_SINT: result = 8; break;
        case VK_FORMAT_R32G32_SFLOAT: result = 8; break;
        case VK_FORMAT_R32G32B32_UINT: result = 12; break;
        case VK_FORMAT_R32G32B32_SINT: result = 12; break;
        case VK_FORMAT_R32G32B32_SFLOAT: result = 12; break;
        case VK_FORMAT_R32G32B32A32_UINT: result = 16; break;
        case VK_FORMAT_R32G32B32A32_SINT: result = 16; break;
        case VK_FORMAT_R32G32B32A32_SFLOAT: result = 16; break;
        case VK_FORMAT_R64_UINT: result = 8; break;
        case VK_FORMAT_R64_SINT: result = 8; break;
        case VK_FORMAT_R64_SFLOAT: result = 8; break;
        case VK_FORMAT_R64G64_UINT: result = 16; break;
        case VK_FORMAT_R64G64_SINT: result = 16; break;
        case VK_FORMAT_R64G64_SFLOAT: result = 16; break;
        case VK_FORMAT_R64G64B64_UINT: result = 24; break;
        case VK_FORMAT_R64G64B64_SINT: result = 24; break;
        case VK_FORMAT_R64G64B64_SFLOAT: result = 24; break;
        case VK_FORMAT_R64G64B64A64_UINT: result = 32; break;
        case VK_FORMAT_R64G64B64A64_SINT: result = 32; break;
        case VK_FORMAT_R64G64B64A64_SFLOAT: result = 32; break;
        case VK_FORMAT_B10G11R11_UFLOAT_PACK32: result = 4; break;
        case VK_FORMAT_E5B9G9R9_UFLOAT_PACK32: result = 4; break;

        default:
            break;
    }
    return result;
}

VkPipelineVertexInputStateCreateInfo PLCore_CreateInputInfoFromShader(PLCore_RenderInstance instance, PLCore_ShaderModule shader) {
    SpvReflectShaderModule mod;
    spvReflectCreateShaderModule(shader.size, shader.buffer, &mod);

    uint32_t inCount = 0;
    uint32_t outputCount = 0;
    spvReflectEnumerateInputVariables(&mod, &inCount, VK_NULL_HANDLE);
    spvReflectEnumerateOutputVariables(&mod, &outputCount, VK_NULL_HANDLE);
    SpvReflectInterfaceVariable** inVariables = malloc(sizeof(SpvReflectInterfaceVariable*) * inCount);
    SpvReflectInterfaceVariable** outVariables = malloc(sizeof(SpvReflectInterfaceVariable*) * inCount);
    spvReflectEnumerateInputVariables(&mod, &inCount, inVariables);
    spvReflectEnumerateOutputVariables(&mod, &outputCount, outVariables);

    uint32_t bindingCount = 1;
    VkVertexInputBindingDescription* binding = malloc(sizeof(VkVertexInputBindingDescription) * bindingCount);
    binding[0].binding = 0;
    binding[0].stride = 0;
    binding[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    VkVertexInputAttributeDescription* attribs = malloc(sizeof(VkVertexInputAttributeDescription) * inCount);
    for (int i = 0; i < inCount; i++) {
        //printf("Shader Input: [\"%s\"]:\n\t Location %i | Format %i\n", inVariables[i]->name, inVariables[i]->location, inVariables[i]->format);
        attribs[i].format = (VkFormat)inVariables[i]->format;
        attribs[i].binding = binding[0].binding;
        attribs[i].location = inVariables[i]->location;
        attribs[i].offset = 0;
    }
    qsort(attribs, inCount, sizeof(VkVertexInputAttributeDescription), compareAttributeDesc);
    for (int i = 0; i < inCount; i++) {
        uint32_t format_size = FormatSize(attribs[i].format);
        attribs[i].offset = binding[0].stride;
        binding[0].stride += format_size;
    }

    VkPipelineVertexInputStateCreateInfo vertexInfo;
    vertexInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInfo.pNext = VK_NULL_HANDLE;
    vertexInfo.flags = 0;
    vertexInfo.pVertexAttributeDescriptions = attribs;
    vertexInfo.vertexAttributeDescriptionCount = inCount;
    vertexInfo.pVertexBindingDescriptions = binding;
    vertexInfo.vertexBindingDescriptionCount = bindingCount;

    return vertexInfo;
}





