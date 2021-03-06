## Devices
Devices is plural because there is 2 kinds: Logical and Physical

Logical devices represent a single instance of the graphics card

Physical devices represent the actual graphics card

## Structures
To create both devices you will need to define some structures

| Structure | Use | Description | Optional |
| -- | -- | -- | -- |
| VkDeviceQueueCreateInfo | Used To Request Queues | Used to signal the device what queues you want to be part of your program | ❔ Somewhat (Not technically required but you will need them for some functions) |
| const char** | Used For Device Extensions | Used to tell the device what extensions you want to be included in your program | ❎ No |
| VkDeviceCreateInfo | Collection Of Structures To Describe How The Device Will Be Made | - | ✔️ Yes |

To create the devices you will first want to create the `VkPhysicalDevice`

```c
    U32 deviceCount = 0;
    vkEnumeratePhysicalDevices(Instance, &deviceCount, VK_NULL_HANDLE);
    // if the function parameter pPhysicalDevices is VK_NULL_HANDLE then the device count will be returned. We can then allocate space for the devices
    VkPhysicalDevice* devices = malloc(sizeof(VkPhysicalDevice) * deviceCount);
    // We will allocate space for all possible graphics cards (likely 1)
    vkEnumeratePhysicalDevices(Instance, &deviceCount, devices);
    // then finally we provide the allocated devices variable into the pPhysicalDevices parameter. This will fill the memory with valid physical devices which we can choose from
    
    VkPhysicalDevice PhysicalDevice;
    
    for (U32 i = 0; i < deviceCount; i++) { // select the best possible graphics card out of the list
        PhysicalDevice = devices[i];
        break; // because we will likely only have 1 we can break the loop immidiately
    }
    free(devices); // not sure if this is good, but it didnt effect my program
```

With our newly found physical device we can now proceed to the logical device

However we will need queues first

To explain queues, I will be showing this image (which is an example, this should not be used for professionals)

![Queue](https://github.com/Bryson-C/Plutonium/blob/main/IfYouUseThisProfessionallyYouAreADumbass_Queues.png)


```c
   // Define this function for readability of queue flags. This is optional though
    void printQueueFlags(VkQueueFamilyProperties family) {
        if (family.queueFlags & VK_QUEUE_GRAPHICS_BIT) { printf(" Graphics "); }
        if (family.queueFlags & VK_QUEUE_SPARSE_BINDING_BIT) { printf(" Sparse Binding "); }
        if (family.queueFlags & VK_QUEUE_TRANSFER_BIT) { printf(" Transfer "); }
        if (family.queueFlags & VK_QUEUE_COMPUTE_BIT) { printf(" Compute "); }
    }
    

    // first we need to find a queue so some functions can be used later
    // much like finding the graphics card the function is called twice, first for the queue family count, next for filling the memory with valid data
    U32 queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(PhysicalDevice, &queueFamilyCount, VK_NULL_HANDLE); 
    // last parameter is VK_NULL_HANDLE so that the function will return the family count
    VkQueueFamilyProperties* queueFamilyProperties = malloc(sizeof(VkQueueFamilyProperties) * queueFamilyCount);
    // allocating space for queue family data
    vkGetPhysicalDeviceQueueFamilyProperties(PhysicalDevice, &queueFamilyCount, queueFamilyProperties);
    // filling queueFamilyProperties with valid queue data
    
    // now we can go through the queue families and select what we need (this is not required, it is used to visualize queue families better)
    for (U32 i = 0; i < queueFamilyCount; i++) {
        printf("Queue Family: %i \n\tFamily Has %i Queues\n\tFamily Is Capable Of: ", i, queueFamilyProperties[i].queueCount);
        printQueueFlags(queueFamilyProperties[i]); // using previously defined function for readability, queueFamilyProperties[i].queueFlags can be printed as a int though
        printf("\n");
    }
    // this will give you a nice view of your graphics cards capabilites of queues
    // we will now select a queue for graphics purposes
    
    U32 GraphicsFamily; // QueueFamily for graphics queue
    U32 GraphicsIndex; // The index of the graphics queue inside of the queue family
    VkQueue GraphicsQueue; // The actual Vulkan queue
    
    for (U32 i = 0; i < queueFamilyCount; i++) {
        if (queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) { // we only need a queue capable of graphics hence why we are only looking for queues with the graphics flag
            GraphicsFamily = i; // we will save the queue family
            GraphicsIndex = 0; // since we only need 1 queue, the index will always be 0
            break; // we are only looking for 1 queue so once we find it we can just exit the loop
        }
    }
    
    free(queueFamilyProperties); // we can now free the queue family properties as we will no longer be using them
```

Finally we will be able to create a logical device

```c
    // we will start out by hard coding our queue info (this is not how I would do it but it is signifigantly easier)
    const float graphicsPriority = 1.0f;

    VkDeviceQueueCreateInfo graphicsInfo;
    graphicsInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    graphicsInfo.pNext = VK_NULL_HANDLE;
    graphicsInfo.flags = 0;
    graphicsInfo.queueFamilyIndex = GraphicsFamily; // the family which the queue belongs to
    graphicsInfo.queueCount = 1; // we only wanted 1 queue from the family
    graphicsInfo.pQueuePriorities = &graphicsPriority; // Im not entirely sure how this works. the Vulkan spec states "normalized floating point values, specifying priorities of work that will be submitted to each created queue"
    
    // next we want extensions and layers for the device (which are also hard coded because I suck at string arrays)
    
    const char* deviceExtensions[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    U32 deviceExtensionCount = 1;
    const char** deviceLayers = VK_NULL_HANDLE;
    U32 deviceLayerCount = 0;
    
    // we may now create the device info
    
    VkDeviceCreateInfo deviceInfo;
    deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceInfo.pNext = VK_NULL_HANDLE;
    deviceInfo.flags = 0;
    deviceInfo.queueCreateInfoCount = 1;
    deviceInfo.pQueueCreateInfos = &graphicsInfo;
    deviceInfo.ppEnabledLayerNames = deviceLayers;
    deviceInfo.enabledLayerCount = deviceLayerCount;
    deviceInfo.enabledExtensionCount = deviceExtensionCount;
    deviceInfo.ppEnabledExtensionNames = deviceExtensions;
    deviceInfo.pEnabledFeatures = VK_NULL_HANDLE; // this will likely be used later I just happen to be lazy
    
    VkDevice Device;
    VkResult DeviceCreationResult = vkCreateDevice(PhysicalDevice, &deviceInfo, VK_NULL_HANDLE, &Device);
    if (DeviceCreationResult != VK_SUCCESS) {} // Do something if the device was created incorrectly (I typical skip this until it becomes a problem)
    
    // the final step is checking if the queue can present and aquiring the queue
    // you will need a VkSurfaceKHR object to check this. This is compeletly optional
    
    vkGetDeviceQueue(Device, GraphicsFamily, GraphicsIndex, &GraphicsQueue);
    
    VkBool32 graphicsQueueCanPresent = VK_FALSE;
    vkGetPhysicalDeviceSurfaceSupportKHR(PhysicalDevice, GraphicsFamily, Surface, &graphicsQueueCanPresent);
    if (graphicsQueueCanPresent != VK_TRUE) {
        printf("Queue Not Capable Of Presentation\n");
    }
    
    
```

