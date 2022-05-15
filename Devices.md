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
| VkDeviceCreateInfo | Collection Of Structures To Describe How The Device Will Be Made | - | ☑️ Yes |
