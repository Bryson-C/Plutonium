# Plutonium - Vulkan In C


## Disclaimer
Im coming from a C++ background but im trying my best to spread information thats helped me. That being said I do not know best practices.
I will also not be explaining build systems because I myself am terrible at them.
I may also forget something or mispell some things so keep that in mind

### **Reminder: Clean the repository. Currently I Just Dump Files However I Feel Like**

**As of 7/24/22 I have started rebuilding the abstractions.h and abstractions.cpp files; The have been moved to the `PlutoniumCore` folder.**

**As of 6/28/22 I have found the descriptor error. The error was occuring due to the descriptor set layout requesting 2 descriptors rather than one.**

**As of 6/15/22 The renderer was switched to a global struct. The structure has everything you need to make the app work with less lines of code while still being able to have access to the objects you need**

**As of 5/29/22 a descriptor set error occurs. This error should not be a problem nor should it harm the application. I do believe that it is an error with lunarg validation layers**

## Libraries
- [GLFW](https://www.glfw.org/)
- [Vulkan SDK](https://vulkan.lunarg.com/) (I will be using Vulkan 1.2; 1.3 should still work though)
- [Wren](https://wren.io/) (Still Needs To Be Worked On)

## Creating Vulkan Objects
- [Creating Instances](https://github.com/Bryson-C/Plutonium/blob/main/Instance.md)
- [Creating Devices And Queues](https://github.com/Bryson-C/Plutonium/blob/main/Devices.md)

## TODO
- Remove Files That Are Outdated Or Deprecated
- Finish Writing About Using Vulkan In C
