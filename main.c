#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#define STB_IMAGE_IMPLEMENTATION 1
#include "lib/stb_image.h"

#define PLCORE_REFLECTION
#include "Abstraction/PlutoniumCore/PlutoniumCore.h"

typedef struct {
    float x;
    float y;
} UNIFORM;


int main() {

    PLCore_RenderInstance RenderInstance = PLCore_CreateRenderingInstance();
    PLCore_Window Window = PLCore_CreateWindow(RenderInstance.pl_instance.instance, 800, 600);
    PLCore_Renderer Renderer = PLCore_CreateRenderer(RenderInstance, Window);


    PLCore_ShaderModule vShader = PLCore_Priv_CreateShader(RenderInstance.pl_device.device, "D:\\Plutonium\\Shaders\\v.spv", VK_SHADER_STAGE_VERTEX_BIT, "main");
    PLCore_ShaderModule fShader = PLCore_Priv_CreateShader(RenderInstance.pl_device.device, "D:\\Plutonium\\Shaders\\f.spv", VK_SHADER_STAGE_FRAGMENT_BIT, "main");



    // TODO: Vertices Are Not Correctly Placed At The Right Coordinants
    PLCore_Vertex vertices[] = {
            {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
            {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
            {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
            {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}}
    };
    uint32_t indices[] = {
            0, 1, 2, 2, 3, 0
    };
    PLCore_Buffer indexBuffer = PLCore_CreateGPUBuffer(RenderInstance, sizeof(uint32_t) * 6, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, indices);

    PLCore_DynamicVertexBuffer dynVertexBuffer = PLCore_CreateDynamicVertexBuffer();
    PLCore_PushVerticesToDynamicVertexBuffer(&dynVertexBuffer, sizeof(PLCore_Vertex), 4, vertices);
    PLCore_Buffer vertexBuffer = PLCore_RequestDynamicVertexBufferToGPU(RenderInstance, &dynVertexBuffer, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, sizeof(PLCore_Vertex));



    PLCore_Buffer uniformBuffers[2] = {
            PLCore_CreateBuffer(RenderInstance, sizeof(UNIFORM), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, CPU_VISIBLE | CPU_COHERENT),
            PLCore_CreateBuffer(RenderInstance, sizeof(UNIFORM), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, CPU_VISIBLE | CPU_COHERENT)
    };




    PLCore_DescriptorPool uniformPool = PLCore_CreateDescriptorPool(RenderInstance, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2);
    PLCore_Descriptor uniformSets = PLCore_CreateDescriptorFromPool(RenderInstance, &uniformPool, 2, 0, 1, VK_SHADER_STAGE_VERTEX_BIT);

    PLCore_UpdateDescriptor(RenderInstance, uniformSets.sets[0], VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, &uniformBuffers[0].bufferInfo, VK_NULL_HANDLE);
    PLCore_UpdateDescriptor(RenderInstance, uniformSets.sets[1], VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, &uniformBuffers[1].bufferInfo, VK_NULL_HANDLE);




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
            }
    };
    VkVertexInputBindingDescription bindings[] = {
            {
                .binding = 0,
                .stride = sizeof(PLCore_Vertex),
                .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
            }
    };
    VkPipelineVertexInputStateCreateInfo vertexInput = PLCore_Priv_CreateVertexInput(3, attribs, 1, bindings);


    PLCore_DescriptorPool samplerPool = PLCore_CreateDescriptorPool(RenderInstance, VK_DESCRIPTOR_TYPE_SAMPLER, 1);
    PLCore_Descriptor samplerSets = PLCore_CreateDescriptorFromPool(RenderInstance, &samplerPool, 1, 0, 1, VK_SHADER_STAGE_FRAGMENT_BIT);
    VkSampler sampler = PLCore_CreateSampler(RenderInstance.pl_device.device, VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_REPEAT);
    VkDescriptorImageInfo samplerInfo = {
            .imageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .imageView = VK_NULL_HANDLE,
            .sampler = sampler,
    };
    PLCore_UpdateDescriptor(RenderInstance, samplerSets.sets[0], VK_DESCRIPTOR_TYPE_SAMPLER, 0, VK_NULL_HANDLE, &samplerInfo);

    const uint32_t MAX_BOUND_IMAGES = 1;
    PLCore_DescriptorPool imagePool = PLCore_CreateDescriptorPool(RenderInstance, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, MAX_BOUND_IMAGES);
    PLCore_Descriptor imageSets = PLCore_CreateDescriptorFromPool(RenderInstance, &imagePool, 1, 0, 1, VK_SHADER_STAGE_FRAGMENT_BIT);
    PLCore_Texture texture = PLCore_CreateTexture(RenderInstance, Renderer, "D:\\Plutonium\\circ.png");
    VkDescriptorImageInfo imageInfo = {
            .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            .imageView = texture.image.view,
            .sampler = VK_NULL_HANDLE,
    };
    PLCore_UpdateDescriptor(RenderInstance, imageSets.sets[0], VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 0, VK_NULL_HANDLE, &imageInfo);

    uint32_t descriptorLayouts = 3;
    VkDescriptorSetLayout layouts[] = {
            uniformSets.layouts[0],
            samplerSets.layouts[0],
            imageSets.layouts[0]
    };

    VkPipelineLayout pipelineLayout = PLCore_Priv_CreatePipelineLayout(RenderInstance.pl_device.device, 0, VK_NULL_HANDLE, descriptorLayouts, layouts);
    PLCore_GraphicsPipeline Pipeline = PLCore_CreatePipeline(RenderInstance, Renderer, vertexInput, vShader, fShader, &pipelineLayout);

    uint32_t fps = 0;
    clock_t timer = clock();
    clock_t buttonCooldown = clock();

    float xPos = 0.0f, yPos = 0.0f;
    clock_t moveTimerX = clock(), moveTimerY = clock();

    while(!glfwWindowShouldClose(Window.window)) {
        // If The Vertex Data Has Been Updated
        if (dynVertexBuffer.dataChanged == 1) {
            vertexBuffer = PLCore_RequestDynamicVertexBufferToGPU(RenderInstance, &dynVertexBuffer, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, sizeof(PLCore_Vertex));
        }

        if (glfwGetKey(Window.window, GLFW_KEY_A) && clock() - moveTimerX > 25) {
            moveTimerX = clock();
            xPos += 0.025f;
        }
        if (glfwGetKey(Window.window, GLFW_KEY_D) && clock() - moveTimerX > 25) {
            moveTimerX =  clock();
            xPos -= 0.025f;
        }
        if (glfwGetKey(Window.window, GLFW_KEY_W) && clock() - moveTimerY > 25) {
            moveTimerY =  clock();
            yPos += 0.025f;
        }
        if (glfwGetKey(Window.window, GLFW_KEY_S) && clock() - moveTimerY > 25) {
            moveTimerY =  clock();
            yPos -= 0.025f;
        }




        glfwPollEvents();

        PLCore_BeginFrame(RenderInstance, &Renderer, &Pipeline, &Window);

        UNIFORM data;
        data.x = xPos;
        data.y = yPos;
        PLCore_UploadDataToBuffer(RenderInstance.pl_device.device, &uniformBuffers[Renderer.priv_activeFrame].memory, sizeof(UNIFORM), &data);




        VkCommandBuffer activeBuffer = PLCore_ActiveRenderBuffer(Renderer);

        uint32_t descriptorSetCount = 3;
        VkDescriptorSet sets[] = {
                uniformSets.sets[Renderer.priv_activeFrame],
                samplerSets.sets[0],
                imageSets.sets[0],
        };
        vkCmdBindDescriptorSets(activeBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, Pipeline.layout, 0, descriptorSetCount, sets, 0, VK_NULL_HANDLE);

        vkCmdBindPipeline(activeBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, Pipeline.pipeline);
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(activeBuffer, 0, 1, &vertexBuffer.buffer, offsets);
        vkCmdBindIndexBuffer(activeBuffer, indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);

        vkCmdDrawIndexed(activeBuffer, 6, 1, 0, 0, 0);


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
