# Plutonium - Vulkan In C (To Be Used As A Learning Tool)

## Disclaimer
Im coming from a C++ background but im trying my best to spread information thats helped me. That being said I do not know best practices.
I will also not be explaining build systems because I myself am terrible at them.
I may also forget something or mispell some things so keep that in mind

**As of 6/15/22 The renderer was switched to a global struct. The structure has everything you need to make the app work with less lines of code while still being able to have access to the objects you need**

**As of 5/29/22 a descriptor set error occurs. This error should not be a problem nor should it harm the application. I do believe that it is an error with lunarg validation layers**

## Libraries
- [GLFW](https://www.glfw.org/)
- [Vulkan SDK](https://vulkan.lunarg.com/) (I will be using Vulkan 1.2; 1.3 should still work though)
 

## Creating Vulkan Objects
- [Creating Instances](https://github.com/Bryson-C/Plutonium/blob/main/Instance.md)
- [Creating Devices And Queues](https://github.com/Bryson-C/Plutonium/blob/main/Devices.md)
