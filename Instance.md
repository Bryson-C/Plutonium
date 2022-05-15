

## Instance
An instance in Vulkan is like the entry point for the graphics api.

To create an instance you need the following information before calling the creation function:

| Structure | Use | Description | Optional |
| -- | -- | -- | -- |
| VkApplicationInfo | Application Information | Used for information such as engine and app name as well as Vulkan version | ❎ No |
| VkInstanceCreateInfo | All Information Structure Bound To Another Structure | Used to attach other information structures such as VkApplicationInfo to be used for creation | ❎ No |
| const char** | Used For Instance Extensions And Layers | Used to be attached to the VkInstanceCreateInfo structure for creation | ❔Somewhat |
| VkDebugUtilsMessengerCreateInfoEXT* | Used For Validation Layers | Used to set up validation layers to debug the entirety of Vulkan during runtime | ✔ Yes (Heavily Suggested) |

To create the instance once all the information has been passed to the `VkInstanceCreateInfo` structure call the `vkCreateInstance` function

```c
   // Note that VK_NULL_HANDLE is the Vulkan handle for NULL/nullptr, I will typically be using VK_NULL_HANDLE

    typedef uint32_t U32;

    VkApplicationInfo appInfo;
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pNext = VK_NULL_HANDLE;
    appInfo.pEngineName = "Engine Name";
    appInfo.engineVersion = VK_MAKE_VERSION(0,1,0); // Can Be A Uint
    appInfo.pApplicationName = "Application Name";
    appInfo.applicationVersion = VK_MAKE_VERSION(0, 1, 0); // Can Be A Uint
    appInfo.apiVersion = VK_API_VERSION_1_2; // Vulkan Version Were Using
   
    // Hard Coded Becuase I Didnt Want To Deal With String Arrays
    U32 extensionCount = 3;
    const char* extensions[] = {VK_KHR_SURFACE_EXTENSION_NAME, VK_EXT_DEBUG_UTILS_EXTENSION_NAME, "VK_KHR_win32_surface"};

    const U32 layerCount = 1;
    const char* layers[] = {"VK_LAYER_KHRONOS_validation"};
    // We only need VK_LAYER_KHRONOS_validation and VK_EXT_DEBUG_UTILS_EXTENSION_NAME if you want to use validation layers
    
    VkInstanceCreateInfo instanceInfo;
    instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceInfo.pNext = VK_NULL_HANDLE; // To Be Used Later For Validation Layers
    instanceInfo.flags = 0;
    instanceInfo.pApplicationInfo = &appInfo;
    instanceInfo.ppEnabledExtensionNames = extensions; // can be VK_NULL_HANDLE or NULL if you dont need extensions
    instanceInfo.enabledExtensionCount = extensionCount; // if extensions are VK_NULL_HANDLE or NULL this must be 0
    instanceInfo.ppEnabledLayerNames = layers; // if layers are VK_NULL_HANDLE or NULL this must be 0
    instanceInfo.enabledLayerCount = layerCount; // can be VK_NULL_HANDLE or NULL if you dont need layers

    
    // You may now finally create your instance
    VkInstance Instance;
    VkResult InstanceCreationResult = vkCreateInstance(&instanceInfo, VK_NULL_HANDLE, &Instance);
    if (InstanceCreationResult != VK_SUCCESS) { 
      // this indicates the instance was built incorrectly, however most of the time (in my expireince it will just throw a runtime error at the creation function)
    }
    
    // you can then destroy the instance with
    vkDestroyInstance(Instance, VK_NULL_HANDLE);
```
