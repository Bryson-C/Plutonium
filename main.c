#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#include <cglm/cglm.h>
#include <cglm/struct.h>

#include "Windows.h"



#define STB_IMAGE_IMPLEMENTATION 1
#include "lib/stb_image.h"

//#define PLCORE_REFLECTION
#include "PlutoniumCore/PlutoniumCore.h"
#include "PlutoniumCore/Descriptors.h"

#define NEW_6INDICES(i) 0 + (i * 4), \
                        1 + (i * 4), \
                        2 + (i * 4), \
                        2 + (i * 4), \
                        3 + (i * 4), \
                        0 + (i * 4)

#define NEW_QUAD(x,y,z, w,h, r,g,b, id)    \
        {{x, y, z},          {r, g, b}, {-1.0f, 0.0f}, id}, \
        {{x + w, y, z},      {r, g, b}, {0.0f, 0.0f},  id}, \
        {{x + w, y + h, z},  {r, g, b}, {0.0f, 1.0f},  id}, \
        {{x, y + h, z},      {r, g, b}, {-1.0f, 1.0f}, id}


#define DEBUG

#ifdef DEBUG
static size_t DEBUG_time = 0;
static char DEBUG_title[256] = "";
#define PLCORE_TIME_TITLED(timerName) \
    strcpy_s(DEBUG_title, 256, timerName); \
    DEBUG_time = clock();


#define PLCORE_TIME() \
    printf("Time Taken For %s: %zi\n", DEBUG_title, clock() - DEBUG_time); \
    DEBUG_time = clock();
#else
#define PLCORE_TIME_TITLED(name)
#define PLCORE_TIME()
#endif

int main() {

    PLCORE_TIME_TITLED("Instance");
    PLCore_RenderInstance RenderInstance = PLCore_CreateRenderingInstance();
    PLCORE_TIME();

    PLCORE_TIME_TITLED("Window");
    PLCore_Window Window = PLCore_CreateWindow(RenderInstance.pl_instance.instance, 800, 600);
    PLCORE_TIME();

    PLCORE_TIME_TITLED("Renderer");
    PLCore_Renderer Renderer = PLCore_CreateRenderer(RenderInstance, Window);
    PLCORE_TIME();

    PLCORE_TIME_TITLED("Vertex Shader");
    PLCore_ShaderModule vShader = PLCore_Priv_CreateShader(RenderInstance.pl_device.device, "D:\\Plutonium\\Shaders\\v.spv", VK_SHADER_STAGE_VERTEX_BIT, "main");
    PLCORE_TIME();

    PLCORE_TIME_TITLED("Fragment Shader");
    PLCore_ShaderModule fShader = PLCore_Priv_CreateShader(RenderInstance.pl_device.device, "D:\\Plutonium\\Shaders\\f.spv", VK_SHADER_STAGE_FRAGMENT_BIT, "main");
    PLCORE_TIME();

    //PLCore_EXP_shaderDescriptors vSets = PLCore_EXP_ReflectShader(RenderInstance, vShader);
    //PLCore_ReflectedDescriptorSet fSets = scanShaders(RenderInstance, fShader);


    // TODO: Vertices Are Not Correctly Placed At The Right Coordinants
    PLCore_Vertex vertices[] = {
            NEW_QUAD(0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1),
            NEW_QUAD(-1.0f, -0.5f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 2),
            NEW_QUAD(-2.0f, -0.5f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 3),
            NEW_QUAD(1.0f, -0.5f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 6),
    };


    uint32_t indexCount = 6 * 5;
    uint32_t indices[] = {
            NEW_6INDICES(0),
            NEW_6INDICES(1),
            NEW_6INDICES(2),
            NEW_6INDICES(3),
            NEW_6INDICES(4),
    };
    PLCORE_TIME_TITLED("Index Buffer");
    PLCore_Buffer indexBuffer = PLCore_CreateGPUBuffer(RenderInstance, sizeof(uint32_t) * indexCount, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, indices);
    PLCORE_TIME()

    PLCORE_TIME_TITLED("Vertex Buffer")
    PLCore_DynamicVertexBuffer dynVertexBuffer = PLCore_CreateDynamicVertexBuffer();
    PLCore_PushVerticesToDynamicVertexBuffer(&dynVertexBuffer, sizeof(PLCore_Vertex), 4 * 4, vertices);
    PLCore_Buffer vertexBuffer = PLCore_RequestDynamicVertexBufferToGPU(RenderInstance, &dynVertexBuffer, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, sizeof(PLCore_Vertex));
    PLCORE_TIME()

    PLCORE_TIME_TITLED("Uniform Buffers")
    PLCore_Buffer cameraUniformBuffers[2] = {
            PLCore_CreateBuffer(RenderInstance, sizeof(PLCore_CameraUniform), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, (VkMemoryPropertyFlagBits)(CPU_VISIBLE | CPU_COHERENT)),
            PLCore_CreateBuffer(RenderInstance, sizeof(PLCore_CameraUniform), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, (VkMemoryPropertyFlagBits)(CPU_VISIBLE | CPU_COHERENT))
    };
    PLCORE_TIME()

    PLCORE_TIME_TITLED("Generalized Descriptor Pool")
    VkDescriptorPool pool = PLCore_CreateGeneralizedDescriptorPool(RenderInstance);
    PLCORE_TIME()

    PLCORE_TIME_TITLED("Camera Descriptor Set")
    VkDescriptorSetLayout cameraLayout[2];
    VkDescriptorSet cameraSets[2] = {
            PLCore_CreateDescriptorSet(RenderInstance, pool, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, &cameraLayout[0]),
            PLCore_CreateDescriptorSet(RenderInstance, pool, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, &cameraLayout[1]),
    };
    PLCORE_TIME()

    PLCORE_TIME_TITLED("Camera Descriptor Set Updates")
    PLCore_UpdateDescriptor(RenderInstance, cameraSets[0], VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, &cameraUniformBuffers[0].bufferInfo, VK_NULL_HANDLE);
    PLCore_UpdateDescriptor(RenderInstance, cameraSets[1], VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, &cameraUniformBuffers[1].bufferInfo, VK_NULL_HANDLE);
    PLCORE_TIME()





    VkVertexInputAttributeDescription attribs[] = {
            {
                .location = 0,
                .binding = 0,
                .format = VK_FORMAT_R32G32B32_SFLOAT,
                .offset = offsetof(PLCore_Vertex, xyz),
            },
            {
                .location = 1,
                .binding = 0,
                .format = VK_FORMAT_R32G32B32_SFLOAT,
                .offset = offsetof(PLCore_Vertex, rgb),
            },
            {
                .location = 2,
                .binding = 0,
                .format = VK_FORMAT_R32G32_SFLOAT,
                .offset = offsetof(PLCore_Vertex, texPos),
            },
            {
                .location = 3,
                .binding = 0,
                .format = VK_FORMAT_R32_UINT,
                .offset = offsetof(PLCore_Vertex, texId),
            }
    };
    VkVertexInputBindingDescription bindings[] = {
            {
                .binding = 0,
                .stride = sizeof(PLCore_Vertex),
                .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
            }
    };
    VkPipelineVertexInputStateCreateInfo vertexInput = PLCore_Priv_CreateVertexInput(4, attribs, 1, bindings);

    PLCORE_TIME_TITLED("Sampler")
    VkSampler sampler = PLCore_CreateSampler(RenderInstance.pl_device.device, VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_REPEAT);
    VkDescriptorImageInfo samplerInfo = {
            .sampler = sampler,
            .imageView = VK_NULL_HANDLE,
            .imageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    };
    PLCORE_TIME()

    PLCORE_TIME_TITLED("Textures")
    #define MAX_BOUND_IMAGES 8


    PLCore_Texture textures[MAX_BOUND_IMAGES] = {
            PLCore_CreateTexture(RenderInstance, Renderer, "D:\\Plutonium\\Assets\\canyon.jpg"),
            PLCore_CreateTexture(RenderInstance, Renderer, "D:\\Plutonium\\Assets\\canyon.jpg"),
            PLCore_CreateTexture(RenderInstance, Renderer, "D:\\Plutonium\\Assets\\jordini.jpg"),
            PLCore_CreateTexture(RenderInstance, Renderer, "D:\\Plutonium\\Assets\\comreezy.jpg"),
            PLCore_CreateTexture(RenderInstance, Renderer, "D:\\Plutonium\\Assets\\can2.jpg"),
            PLCore_CreateTexture(RenderInstance, Renderer, "D:\\Plutonium\\Assets\\dave.jpg"),
            PLCore_CreateTexture(RenderInstance, Renderer, "D:\\Plutonium\\Assets\\doggy.png"),
            PLCore_CreateTexture(RenderInstance, Renderer, "D:\\Plutonium\\Assets\\texture.jpg"),
    };
    PLCORE_TIME()

    PLCORE_TIME_TITLED("Texture Bindings")
    VkDescriptorSetLayoutBinding textureBindings[] = {
            PLCore_CreateDescriptorSetLayoutBinding(0, 1, VK_DESCRIPTOR_TYPE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT),
            PLCore_CreateDescriptorSetLayoutBinding(1, 8, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_FRAGMENT_BIT),
    };
    PLCORE_TIME()

    PLCORE_TIME_TITLED("Texture Descriptor Sets")
    VkDescriptorSetLayout textureLayout;
    VkDescriptorSet textureSet[] = {
            PLCore_CreateDescriptorSetAdvanced(RenderInstance, pool, 2, textureBindings, VK_SHADER_STAGE_FRAGMENT_BIT, &textureLayout),
    };
    VkDescriptorImageInfo textureInfo[MAX_BOUND_IMAGES];
    for (int i = 0; i < MAX_BOUND_IMAGES; i++) {
        textureInfo[i] = textures[i].imageInfo;
    }
    PLCORE_TIME()

    PLCORE_TIME_TITLED("Texture Descriptor Set Update")
    VkWriteDescriptorSet write[] = {
            {
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .pNext = VK_NULL_HANDLE,
                .dstSet = textureSet[0],
                .dstBinding = 0,
                .dstArrayElement = 0,
                .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER,
                .descriptorCount = 1,
                .pImageInfo = &samplerInfo,
                .pBufferInfo = VK_NULL_HANDLE,
                .pTexelBufferView = VK_NULL_HANDLE,
            },
            {
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .pNext = VK_NULL_HANDLE,
                .dstSet = textureSet[0],
                .dstBinding = 1,
                .dstArrayElement = 0,
                .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                .descriptorCount = MAX_BOUND_IMAGES,
                .pImageInfo = textureInfo,
                .pBufferInfo = VK_NULL_HANDLE,
                .pTexelBufferView = VK_NULL_HANDLE,
            },
    };
    vkUpdateDescriptorSets(RenderInstance.pl_device.device, 2, write, 0, VK_NULL_HANDLE);
    PLCORE_TIME()

    uint32_t descriptorLayouts = 2;
    VkDescriptorSetLayout layouts[] = {
            cameraLayout[0],
            textureLayout,
    };

    PLCORE_TIME_TITLED("Pipeline")
    VkPipelineLayout pipelineLayout = PLCore_Priv_CreatePipelineLayout(RenderInstance.pl_device.device, 0, VK_NULL_HANDLE, descriptorLayouts, layouts);
    PLCore_GraphicsPipeline Pipeline = PLCore_CreatePipeline(RenderInstance, Renderer, vertexInput, vShader, fShader, &pipelineLayout);
    PLCORE_TIME()

    uint32_t fps = 0;
    clock_t timer = clock();


    PLCORE_TIME_TITLED("Movement Controller")
    PLCore_CameraMoveScheme keybindingScheme = PLCore_GetDefaultMoveScheme();
    PLCore_CameraUniform camera = PLCore_CreateCameraUniform();
    PLCORE_TIME()

    clock_t playerMoveTime = clock();
    while(!glfwWindowShouldClose(Window.window)) {

        if (clock() - playerMoveTime >= 16) {
            playerMoveTime = clock();
            VkCommandBuffer updateBuffer = PLCore_Priv_CreateCommandBuffers(RenderInstance.pl_device.device, Renderer.graphicsPool.pool, 1)[0];
            PLCore_RecordCommandBuffer(updateBuffer);

            PLCore_Vertex *updated = vertices + 8;
            if (glfwGetKey(Window.window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
                (*(updated + 0)).xyz[0] += 0.25f;
                (*(updated + 1)).xyz[0] += 0.25f;
                (*(updated + 2)).xyz[0] += 0.25f;
                (*(updated + 3)).xyz[0] += 0.25f;
            }
            if (glfwGetKey(Window.window, GLFW_KEY_LEFT) == GLFW_PRESS) {
                (*(updated + 0)).xyz[0] -= 0.25f;
                (*(updated + 1)).xyz[0] -= 0.25f;
                (*(updated + 2)).xyz[0] -= 0.25f;
                (*(updated + 3)).xyz[0] -= 0.25f;
            }
            vkCmdUpdateBuffer(updateBuffer, vertexBuffer.buffer, sizeof(PLCore_Vertex) * 8, sizeof(PLCore_Vertex) * 8, updated);

            PLCore_StopCommandBuffer(updateBuffer);
            PLCore_SubmitCommandBuffer(updateBuffer, RenderInstance.pl_device.graphicsQueue.queue, VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE);

        }
        // If The Vertex Data Has Been Updated
        if (dynVertexBuffer.dataChanged == 1) {
            // Wait For Both Fences Because Both Frames Might Still Be In Use
            vkWaitForFences(RenderInstance.pl_device.device, 2, Renderer.priv_renderFences, VK_TRUE, UINT64_MAX);
            vertexBuffer = PLCore_RequestDynamicVertexBufferToGPU(RenderInstance, &dynVertexBuffer, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, sizeof(PLCore_Vertex));
        }

        glfwPollEvents();
        PLCore_PollCameraMovements(Window, &camera, keybindingScheme);
        PLCore_UploadDataToBuffer(RenderInstance.pl_device.device,
                                  &cameraUniformBuffers[Renderer.priv_activeFrame].memory,
                                  sizeof(PLCore_CameraUniform),
                                  &camera);

        PLCore_BeginFrame(RenderInstance, &Renderer, &Pipeline, &Window);
        VkCommandBuffer activeBuffer = PLCore_ActiveRenderBuffer(Renderer);


        uint32_t descriptorSetCount = 2;
        VkDescriptorSet bindSets[] = {
                cameraSets[Renderer.priv_activeFrame],
                textureSet[0],
        };
        vkCmdBindDescriptorSets(activeBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, Pipeline.layout, 0, descriptorSetCount, bindSets, 0, VK_NULL_HANDLE);

        vkCmdBindPipeline(activeBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, Pipeline.pipeline);
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(activeBuffer, 0, 1, &vertexBuffer.buffer, offsets);
        vkCmdBindIndexBuffer(activeBuffer, indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);

        vkCmdDrawIndexed(activeBuffer, indexCount, 1, 0, 0, 0);

        PLCore_EndFrame(RenderInstance, &Renderer, &Pipeline, &Window);
        glfwSwapBuffers(Window.window);






        fps++;
        if (clock()-timer > 1000) {
            printf("Drawing %zi Vertices  |  ", dynVertexBuffer.dataCount);
            timer = clock();
            printf("FPS: %u\n", fps);
            fps = 0;
        }
    }


    glfwTerminate();

    return 0;

}
