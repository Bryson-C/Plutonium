//
// Created by Owner on 6/23/2022.
//

#include "PlutoniumCore.h"


static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback( VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
    if (messageSeverity > VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) printf("[Vk]: %s\n\n",pCallbackData->pMessage);
    return VK_FALSE;
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

PLCore_RenderInstance PLCore_CreateRenderingInstance() {
    PLCore_RenderInstance renderInstance;
    renderInstance.instance.instance = PLCore_Priv_CreateInstance(&renderInstance.instance.priv_Messenger);

    VkQueueFamilyProperties* queueFamilyProperties;
    uint32_t queueFamilyCount;

    renderInstance.device.physicalDevice = PLCore_Priv_CreatePhysicalDevice(renderInstance.instance.instance, &queueFamilyCount, &queueFamilyProperties);
    vkGetPhysicalDeviceProperties(renderInstance.device.physicalDevice, &renderInstance.device.properties);
    vkGetPhysicalDeviceFeatures(renderInstance.device.physicalDevice, &renderInstance.device.features);
    vkGetPhysicalDeviceMemoryProperties(renderInstance.device.physicalDevice, &renderInstance.device.memoryProperties);

    PLCore_DeviceQueue* queues = VK_NULL_HANDLE;
    uint32_t queueRequestCount = 2;
    VkQueueFlagBits queueRequest[] = {VK_QUEUE_GRAPHICS_BIT, VK_QUEUE_TRANSFER_BIT};
    renderInstance.device.device = PLCore_Priv_CreateDevice(renderInstance.device.physicalDevice, queueFamilyCount, queueFamilyProperties, renderInstance.device.features, queueRequestCount, queueRequest, &queues);

    return renderInstance;
}