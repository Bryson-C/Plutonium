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
    // first we need to find a queue so some functions can be used later
    // much like finding the graphics card the function is called twice, first for the queue family count, next for filling the memory with valid data
    U32 queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(PhysicalDevice, &queueFamilyCount, VK_NULL_HANDLE);
    VkQueueFamilyProperties* queueFamilyProperties = malloc(sizeof(VkQueueFamilyProperties) * queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(PhysicalDevice, &queueFamilyCount, queueFamilyProperties);
```
