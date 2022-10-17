#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#include <cglm/cglm.h>
#include <cglm/struct.h>

#define STB_IMAGE_IMPLEMENTATION 1
#include "lib/stb_image.h"

//#define PLCORE_REFLECTION
#include "PlutoniumCore/PlutoniumCore.h"

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

#include "PlutoniumCore/ShaderReflection.h"






int main() {

    PLCore_RenderInstance RenderInstance = PLCore_CreateRenderingInstance();
    PLCore_Window Window = PLCore_CreateWindow(RenderInstance.pl_instance.instance, 800, 600);
    PLCore_Renderer Renderer = PLCore_CreateRenderer(RenderInstance, Window);

    // TODO: Fix: These Send Un-Initialized Data, Thereby Throwing A Runtime Error
    PLCore_ShaderModule vShader = PLCore_Priv_CreateShader(RenderInstance.pl_device.device, "D:\\Plutonium\\Shaders\\v.spv", VK_SHADER_STAGE_VERTEX_BIT, "main");
    PLCore_ShaderModule fShader = PLCore_Priv_CreateShader(RenderInstance.pl_device.device, "D:\\Plutonium\\Shaders\\f.spv", VK_SHADER_STAGE_FRAGMENT_BIT, "main");

    //DescriptorReturnData vDescriptorData = scanShader(RenderInstance, vShader);

    //DescriptorReturnData fDescriptorData = scanShader(RenderInstance, fShader);



    // TODO: Vertices Are Not Correctly Placed At The Right Coordinants
    PLCore_Vertex vertices[] = {
            NEW_QUAD(0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1),
            NEW_QUAD(-0.5f, -0.5f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 2),
    };
    uint32_t indexCount = 6 * 5;
    uint32_t indices[] = {
            NEW_6INDICES(0),
            NEW_6INDICES(1),
            NEW_6INDICES(2),
            NEW_6INDICES(3),
            NEW_6INDICES(4),
    };
    PLCore_Buffer indexBuffer = PLCore_CreateGPUBuffer(RenderInstance, sizeof(uint32_t) * indexCount, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, indices);

    PLCore_DynamicVertexBuffer dynVertexBuffer = PLCore_CreateDynamicVertexBuffer();
    PLCore_PushVerticesToDynamicVertexBuffer(&dynVertexBuffer, sizeof(PLCore_Vertex), 4 * 2, vertices);
    PLCore_Buffer vertexBuffer = PLCore_RequestDynamicVertexBufferToGPU(RenderInstance, &dynVertexBuffer, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, sizeof(PLCore_Vertex));



    PLCore_Buffer cameraUniformBuffers[2] = {
            PLCore_CreateBuffer(RenderInstance, sizeof(PLCore_CameraUniform), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, CPU_VISIBLE | CPU_COHERENT),
            PLCore_CreateBuffer(RenderInstance, sizeof(PLCore_CameraUniform), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, CPU_VISIBLE | CPU_COHERENT)
    };

    //PLCore_UpdateDescriptor(RenderInstance, vDescriptorData.descriptors[0].sets[0], VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, &cameraUniformBuffers[0].bufferInfo, VK_NULL_HANDLE);


    PLCore_DescriptorPool uniformPool = PLCore_CreateDescriptorPool(RenderInstance, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2);
    PLCore_Descriptor uniformSets = PLCore_CreateDescriptorFromPool(RenderInstance, &uniformPool, 2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, 1, VK_SHADER_STAGE_VERTEX_BIT);
    PLCore_UpdateDescriptor(RenderInstance, uniformSets.sets[0], VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, &cameraUniformBuffers[0].bufferInfo, VK_NULL_HANDLE);
    PLCore_UpdateDescriptor(RenderInstance, uniformSets.sets[1], VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, &cameraUniformBuffers[1].bufferInfo, VK_NULL_HANDLE);





    VkVertexInputAttributeDescription attribs[] = {
            {
                .binding = 0,
                .format = VK_FORMAT_R32G32B32_SFLOAT,
                .offset = offsetof(PLCore_Vertex, xyz),
                .location = 0,
            },
            {
                .binding = 0,
                .format = VK_FORMAT_R32G32B32_SFLOAT,
                .offset = offsetof(PLCore_Vertex, rgb),
                .location = 1,
            },
            {
                .binding = 0,
                .format = VK_FORMAT_R32G32_SFLOAT,
                .offset = offsetof(PLCore_Vertex, texPos),
                .location = 2,
            },
            {
                .binding = 0,
                .format = VK_FORMAT_R32_UINT,
                .offset = offsetof(PLCore_Vertex, texId),
                .location = 3,
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

    PLCore_DescriptorPool samplerPool = PLCore_CreateDescriptorPool(RenderInstance, VK_DESCRIPTOR_TYPE_SAMPLER, 2);
    PLCore_Descriptor samplerSets = PLCore_CreateDescriptorFromPool(RenderInstance, &samplerPool, 2, VK_DESCRIPTOR_TYPE_SAMPLER, 0, 1, VK_SHADER_STAGE_FRAGMENT_BIT);
    VkSampler sampler = PLCore_CreateSampler(RenderInstance.pl_device.device, VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_REPEAT);
    VkDescriptorImageInfo samplerInfo = {
            .imageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .imageView = VK_NULL_HANDLE,
            .sampler = sampler,
    };
    PLCore_UpdateDescriptor(RenderInstance, samplerSets.sets[0], VK_DESCRIPTOR_TYPE_SAMPLER, 0, VK_NULL_HANDLE, &samplerInfo);
    PLCore_UpdateDescriptor(RenderInstance, samplerSets.sets[1], VK_DESCRIPTOR_TYPE_SAMPLER, 0, VK_NULL_HANDLE, &samplerInfo);

    const uint32_t MAX_BOUND_IMAGES = 8;
    PLCore_DescriptorPool imagePool = PLCore_CreateDescriptorPool(RenderInstance, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, MAX_BOUND_IMAGES);
    PLCore_Descriptor imageSets = PLCore_CreateDescriptorFromPool(RenderInstance, &imagePool, 1, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 0, 8, VK_SHADER_STAGE_FRAGMENT_BIT);
    PLCore_Texture textures[] = {
            PLCore_CreateTexture(RenderInstance, Renderer, "D:\\Plutonium\\canyon.jpg"),
            PLCore_CreateTexture(RenderInstance, Renderer, "D:\\Plutonium\\canyon.jpg"),
            PLCore_CreateTexture(RenderInstance, Renderer, "D:\\Plutonium\\jordini.jpg"),
            PLCore_CreateTexture(RenderInstance, Renderer, "D:\\Plutonium\\comreezy.jpg"),
            PLCore_CreateTexture(RenderInstance, Renderer, "D:\\Plutonium\\can2.jpg"),
            PLCore_CreateTexture(RenderInstance, Renderer, "D:\\Plutonium\\dave.jpg"),
            PLCore_CreateTexture(RenderInstance, Renderer, "D:\\Plutonium\\texture.jpg"),
            PLCore_CreateTexture(RenderInstance, Renderer, "D:\\Plutonium\\texture.jpg"),
    };

    VkDescriptorImageInfo imageInfos[8];
    for (int32_t i = 0; i < MAX_BOUND_IMAGES; i++) {
        imageInfos[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfos[i].imageView = textures[i].image.view;
        imageInfos[i].sampler = VK_NULL_HANDLE;
    }

    VkWriteDescriptorSet write = {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .pNext = VK_NULL_HANDLE,
            .dstSet = imageSets.sets[0],
            .dstBinding = 0,
            .dstArrayElement = 0,
            .descriptorCount = MAX_BOUND_IMAGES,
            .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
            .pImageInfo = imageInfos,
            .pBufferInfo = VK_NULL_HANDLE,
            .pTexelBufferView = VK_NULL_HANDLE,
    };
    vkUpdateDescriptorSets(RenderInstance.pl_device.device, 1, &write, 0, VK_NULL_HANDLE);


    uint32_t descriptorLayouts = 3;
    VkDescriptorSetLayout layouts[] = {
            uniformSets.layouts[0],
            //vDescriptorData.descriptors[0].layouts[0],
            samplerSets.layouts[0],
            imageSets.layouts[0]
    };

    VkPipelineLayout pipelineLayout = PLCore_Priv_CreatePipelineLayout(RenderInstance.pl_device.device, 0, VK_NULL_HANDLE, descriptorLayouts, layouts);
    PLCore_GraphicsPipeline Pipeline = PLCore_CreatePipeline(RenderInstance, Renderer, vertexInput, vShader, fShader, &pipelineLayout);

    uint32_t fps = 0;
    clock_t timer = clock();

    bool isBuddyHere = false;

    PLCore_CameraUniform camera = PLCore_CreateCameraUniform();
    while(!glfwWindowShouldClose(Window.window)) {

        if (glfwGetMouseButton(Window.window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS && !isBuddyHere) {
            isBuddyHere = true;
            PLCore_Vertex newVertices[] = {NEW_QUAD(0.0f, 1.0f, 0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 3)};
            PLCore_PushVerticesToDynamicVertexBuffer(&dynVertexBuffer, sizeof(PLCore_Vertex), 4, newVertices);
        }

        // If The Vertex Data Has Been Updated
        if (dynVertexBuffer.dataChanged == 1) {
            vertexBuffer = PLCore_RequestDynamicVertexBufferToGPU(RenderInstance, &dynVertexBuffer, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, sizeof(PLCore_Vertex));
        }

        PLCore_CameraMoveScheme scheme = PLCore_GetDefaultMoveScheme();
        glfwPollEvents();
        PLCore_PollCameraMovements(Window, &camera, scheme);

        PLCore_UploadDataToBuffer(RenderInstance.pl_device.device, &cameraUniformBuffers[Renderer.priv_activeFrame].memory, sizeof(PLCore_CameraUniform), &camera);


        PLCore_BeginFrame(RenderInstance, &Renderer, &Pipeline, &Window);

        VkCommandBuffer activeBuffer = PLCore_ActiveRenderBuffer(Renderer);

        uint32_t descriptorSetCount = 3;
        VkDescriptorSet sets[] = {
                uniformSets.sets[Renderer.priv_activeFrame],
                //vDescriptorData.descriptors[0].sets[0],
                samplerSets.sets[Renderer.priv_activeFrame],
                imageSets.sets[0],
        };
        vkCmdBindDescriptorSets(activeBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, Pipeline.layout, 0, descriptorSetCount, sets, 0, VK_NULL_HANDLE);

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
