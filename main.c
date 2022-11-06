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





int main() {

    PLCore_RenderInstance RenderInstance = PLCore_CreateRenderingInstance();
    PLCore_Window Window = PLCore_CreateWindow(RenderInstance.pl_instance.instance, 800, 600);
    PLCore_Renderer Renderer = PLCore_CreateRenderer(RenderInstance, Window);

    PLCore_ShaderModule vShader = PLCore_Priv_CreateShader(RenderInstance.pl_device.device, "D:\\Plutonium\\Shaders\\v.spv", VK_SHADER_STAGE_VERTEX_BIT, "main");
    PLCore_ShaderModule fShader = PLCore_Priv_CreateShader(RenderInstance.pl_device.device, "D:\\Plutonium\\Shaders\\f.spv", VK_SHADER_STAGE_FRAGMENT_BIT, "main");


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
    PLCore_Buffer indexBuffer = PLCore_CreateGPUBuffer(RenderInstance, sizeof(uint32_t) * indexCount, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, indices);

    PLCore_DynamicVertexBuffer dynVertexBuffer = PLCore_CreateDynamicVertexBuffer();
    PLCore_PushVerticesToDynamicVertexBuffer(&dynVertexBuffer, sizeof(PLCore_Vertex), 4 * 4, vertices);
    PLCore_Buffer vertexBuffer = PLCore_RequestDynamicVertexBufferToGPU(RenderInstance, &dynVertexBuffer, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, sizeof(PLCore_Vertex));

    PLCore_Buffer cameraUniformBuffers[2] = {
            PLCore_CreateBuffer(RenderInstance, sizeof(PLCore_CameraUniform), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, (VkMemoryPropertyFlagBits)(CPU_VISIBLE | CPU_COHERENT)),
            PLCore_CreateBuffer(RenderInstance, sizeof(PLCore_CameraUniform), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, (VkMemoryPropertyFlagBits)(CPU_VISIBLE | CPU_COHERENT))
    };


    VkDescriptorPool pool = PLCore_CreateGeneralizedDescriptorPool(RenderInstance);
    VkDescriptorSetLayout cameraLayout[2];
    VkDescriptorSet cameraSets[2] = {
            PLCore_CreateDescriptorSet(RenderInstance, pool, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, &cameraLayout[0]),
            PLCore_CreateDescriptorSet(RenderInstance, pool, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, &cameraLayout[1]),
    };
    PLCore_UpdateDescriptor(RenderInstance.pl_device.device, cameraSets[0], VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, &cameraUniformBuffers[0].bufferInfo, VK_NULL_HANDLE);
    PLCore_UpdateDescriptor(RenderInstance.pl_device.device, cameraSets[1], VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, &cameraUniformBuffers[1].bufferInfo, VK_NULL_HANDLE);






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

    VkSampler sampler = PLCore_CreateSampler(RenderInstance.pl_device.device, VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_REPEAT);
    VkDescriptorImageInfo samplerInfo = {
            .sampler = sampler,
            .imageView = VK_NULL_HANDLE,
            .imageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    };

    //PLCore_UpdateDescriptor(RenderInstance.pl_device.device, fSets.sets[0]->set, VK_DESCRIPTOR_TYPE_SAMPLER, 0, VK_NULL_HANDLE, &samplerInfo);


    // TODO: This Crashes The Program
    // PLCore_UpdateDescriptor(RenderInstance.pl_device.device, samplerSets[0].set, VK_DESCRIPTOR_TYPE_SAMPLER, 0, VK_NULL_HANDLE, &samplerInfo);
    // PLCore_UpdateDescriptor(RenderInstance.pl_device.device, samplerSets[1].set, VK_DESCRIPTOR_TYPE_SAMPLER, 0, VK_NULL_HANDLE, &samplerInfo);




    const uint32_t MAX_BOUND_IMAGES = 8;
    VkDescriptorType imagePoolTypes[] = {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE};
    uint32_t imagePoolCounts[] = {MAX_BOUND_IMAGES};
    PLCore_DescriptorPoolAllocator imagePool = PLCore_CreateDescriptorPoolFromAllocator(
            RenderInstance.pl_device.device,
            PLCore_CreateDescriptorPoolAllocator(2, imagePoolTypes, imagePoolCounts, 1, MAX_BOUND_IMAGES, VK_SHADER_STAGE_FRAGMENT_BIT));

    PLCore_DescriptorSet imageSets[] = {
            PLCore_CreateDescriptorSets(RenderInstance.pl_device.device, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, imagePool),
    };

    PLCore_Texture textures[] = {
            PLCore_CreateTexture(RenderInstance, Renderer, "D:\\Plutonium\\Assets\\canyon.jpg"),
            PLCore_CreateTexture(RenderInstance, Renderer, "D:\\Plutonium\\Assets\\canyon.jpg"),
            PLCore_CreateTexture(RenderInstance, Renderer, "D:\\Plutonium\\Assets\\jordini.jpg"),
            PLCore_CreateTexture(RenderInstance, Renderer, "D:\\Plutonium\\Assets\\comreezy.jpg"),
            PLCore_CreateTexture(RenderInstance, Renderer, "D:\\Plutonium\\Assets\\can2.jpg"),
            PLCore_CreateTexture(RenderInstance, Renderer, "D:\\Plutonium\\Assets\\dave.jpg"),
            PLCore_CreateTexture(RenderInstance, Renderer, "D:\\Plutonium\\Assets\\self.jpg"),
            PLCore_CreateTexture(RenderInstance, Renderer, "D:\\Plutonium\\Assets\\texture.jpg"),
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
            .dstSet = imageSets[0].set,
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
            cameraLayout[0],
            //vSets.layouts[0][0],
            //fSets.sets[0]->layout,
            //imageSets[0].layout
    };

    VkPipelineLayout pipelineLayout = PLCore_Priv_CreatePipelineLayout(RenderInstance.pl_device.device, 0, VK_NULL_HANDLE, descriptorLayouts, layouts);
    PLCore_GraphicsPipeline Pipeline = PLCore_CreatePipeline(RenderInstance, Renderer, vertexInput, vShader, fShader, &pipelineLayout);

    uint32_t fps = 0;
    clock_t timer = clock();


    PLCore_CameraUniform camera = PLCore_CreateCameraUniform();
    while(!glfwWindowShouldClose(Window.window)) {

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

        uint32_t descriptorSetCount = 1;
        VkDescriptorSet bindSets[] = {
                cameraSets[Renderer.priv_activeFrame],
                //vSets.sets[0][0],
                //fSets.sets[0]->set,
                //imageSets[0].set,
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
