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

#include "lib/GL/gl.h"
#include "lib/GLFW/include/glfw3.h"
#include "PlutoniumCore/PlutoniumCore.h"
#include "PlutoniumCore/Engine.h"

/*
int main(int argc, char** argv) {
    WS_Engine engine = WS_Initialize(800, 600);

    VkDescriptorPool pool = PLCore_CreateGeneralizedDescriptorPool(engine.instance);

    PLCore_ShaderModule vertexShader = PLCore_CreateShader(engine.instance,"D:\\Plutonium\\Shaders\\v.spv", VK_SHADER_STAGE_VERTEX_BIT, "main", pool);

    PLCore_CreateInputInfoFromShader(engine.instance, vertexShader);

    PLCore_ShaderModule fragmentShader = PLCore_CreateShader(engine.instance, "D:\\Plutonium\\Shaders\\f.spv", VK_SHADER_STAGE_FRAGMENT_BIT, "main", pool);

    const uint32_t framesInFlight = 2; // this is a constant (it should be at least) it needs to be at 2
    PLCore_Buffer uniformBuffers[] = {
            PLCore_CreateUniformBuffer(engine.instance, sizeof(PLCore_CameraUniform)),
            PLCore_CreateUniformBuffer(engine.instance, sizeof(PLCore_CameraUniform)),
    };
    vertexShader.descriptorSets[0].writes[0].pBufferInfo = &uniformBuffers[0].bufferInfo;
    vkUpdateDescriptorSets(engine.instance.pl_device.device, 1, vertexShader.descriptorSets[0].writes, 0, VK_NULL_HANDLE);

    PLCore_ImageSampler sampler = PLCore_CreateSampler(engine.instance.pl_device.device, VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_REPEAT);
    fragmentShader.descriptorSets[0].writes[0].pImageInfo = &sampler.samplerInfo;

    #define MAX_BOUND_IMAGES 8
    PLCore_Texture textures[MAX_BOUND_IMAGES] = {
            PLCore_CreateTexture(engine.instance, engine.renderer, "D:\\Plutonium\\Assets\\canyon.jpg"),
            PLCore_CreateTexture(engine.instance, engine.renderer, "D:\\Plutonium\\Assets\\canyon.jpg"),
            PLCore_CreateTexture(engine.instance, engine.renderer, "D:\\Plutonium\\Assets\\jordini.jpg"),
            PLCore_CreateTexture(engine.instance, engine.renderer, "D:\\Plutonium\\Assets\\comreezy.jpg"),
            PLCore_CreateTexture(engine.instance, engine.renderer, "D:\\Plutonium\\Assets\\can2.jpg"),
            PLCore_CreateTexture(engine.instance, engine.renderer, "D:\\Plutonium\\Assets\\dave.jpg"),
            PLCore_CreateTexture(engine.instance, engine.renderer, "D:\\Plutonium\\Assets\\doggy.png"),
            PLCore_CreateTexture(engine.instance, engine.renderer, "D:\\Plutonium\\Assets\\texture.jpg"),
    };
    VkDescriptorImageInfo textureInfo[MAX_BOUND_IMAGES];
    for (int i = 0; i < MAX_BOUND_IMAGES; i++) {
        textureInfo[i] = textures[i].imageInfo;
    }
    fragmentShader.descriptorSets[0].writes[1].pImageInfo = textureInfo;

    vkUpdateDescriptorSets(engine.instance.pl_device.device, 2, fragmentShader.descriptorSets[0].writes, 0, VK_NULL_HANDLE);

    engine = WS_InitializeRenderer(engine, vertexShader, fragmentShader);

    engine.priv_vertObj.vertices = realloc(engine.priv_vertObj.vertices, sizeof(PLCore_Vertex) * 4);
    PLCore_Vertex vertices[] = {
            NEW_QUAD(-0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1),
    };
    for (int i = 0; i < 4; i++)  {
        engine.priv_vertObj.vertices[i] = vertices[i];
    }

    engine.priv_vertObj.vertexBuffer = PLCore_CreateGPUBuffer(engine.instance,
                                                              sizeof(PLCore_Vertex) * 4,
                                                              VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                                                              engine.priv_vertObj.vertices);
    uint32_t indexCount = 6;
    uint32_t indices[] =
            {
                0, 1, 2, 2, 3, 0,
            };
    PLCore_Buffer indexBuffer = PLCore_CreateGPUBuffer(engine.instance, sizeof(uint32_t) * indexCount, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, indices);

    PLCore_CameraMoveScheme keyBinds = PLCore_GetDefaultMoveScheme();
    PLCore_CameraUniform camera = PLCore_CreateCameraUniform();

    clock_t time = clock();

    while (WS_StartRenderLoop(&engine)) {
        PLCore_PollCameraMovements(engine.window, &camera, keyBinds);
        PLCore_UploadDataToBuffer(engine.instance.pl_device.device, &uniformBuffers[engine.renderer.priv_activeFrame].memory, sizeof(PLCore_CameraUniform), &camera);

        if (clock() - time > 300) {
            WS_NewQuad(&engine, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 4);
        }


        VkCommandBuffer activeBuffer = PLCore_ActiveRenderBuffer(engine.renderer);
        WS_BindDescriptors(&engine);

        vkCmdBindIndexBuffer(activeBuffer, indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
        vkCmdDrawIndexed(activeBuffer, indexCount, 1, 0, 0, 0);

        WS_StopRenderLoop(&engine);
    }
}
*/

#include "PlutoniumCore/ScriptCore.h"

int main() {

    parseScript("D:\\Plutonium\\scripts\\test.events");


    Engine engine;
    ReturnResult initResult = WS_Start(&engine, "D:\\Plutonium\\Shaders\\v.spv", "D:\\Plutonium\\Shaders\\f.spv", VK_NULL_HANDLE);


    PLCore_CameraUniform camera = PLCore_CreateCameraUniform();
    PLCore_CameraMoveScheme keybinds = PLCore_GetDefaultMoveScheme();


    PLCore_Buffer uniformBuffers[] = {
            PLCore_CreateUniformBuffer(engine.instance, sizeof(PLCore_CameraUniform)),
            PLCore_CreateUniformBuffer(engine.instance, sizeof(PLCore_CameraUniform)),
    };

    PLCore_ImageSampler sampler = PLCore_CreateSampler(engine.instance.pl_device.device, VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_REPEAT);
    PLCore_Texture textures[] = {
            PLCore_CreateTexture(engine.instance, engine.renderer, "D:\\Plutonium\\Assets\\ps5.jpg")
    };

/*
    VkWriteDescriptorSet writes[2];
    writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writes[0].pNext = VK_NULL_HANDLE;
    writes[0].dstSet = engine.descriptorSets[1];
    writes[0].dstBinding = 0;
    writes[0].dstArrayElement = 0;
    writes[0].descriptorCount = 1;
    writes[0].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
    writes[0].pImageInfo = &sampler.samplerInfo;
    writes[0].pBufferInfo = VK_NULL_HANDLE;
    writes[0].pTexelBufferView = VK_NULL_HANDLE;

    writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writes[1].pNext = VK_NULL_HANDLE;
    writes[1].dstSet = engine.descriptorSets[1];
    writes[1].dstBinding = 1;
    writes[1].dstArrayElement = 0;
    writes[1].descriptorCount = 1;
    writes[1].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    writes[1].pImageInfo = &textures->imageInfo;
    writes[1].pBufferInfo = VK_NULL_HANDLE;
    writes[1].pTexelBufferView = VK_NULL_HANDLE;

    //WS_UniformW(&engine, "textures", writes, 2);
    vkUpdateDescriptorSets(engine.instance.pl_device.device, 2, writes, 0, VK_NULL_HANDLE);
*/
    WS_UniformI(&engine, "textureSampler", sampler.samplerInfo, VK_NULL_HANDLE);
    WS_UniformI(&engine, "textures", textures->imageInfo, VK_NULL_HANDLE);

    // Make Texture Work!

    PLCore_Vertex vertices[3] = {
            { { -0.5f, -0.5f,  0.0f }, { 1.0f, 0.0f, 0.0f }, {0.0f, 0.0f}, 1},
            { { -0.5f,  0.5f,  0.0f }, { 0.0f, 1.0f, 0.0f }, {0.0f, 0.0f}, 1},
            { {  0.5f,  0.5f,  0.0f }, { 0.0f, 0.0f, 1.0f }, {0.0f, 0.0f}, 1}
    };
    WS_NewGeometry(&engine, vertices, 3);

    PLCore_PollCameraMovements(engine.window, &camera, keybinds);

    VkDescriptorBufferInfo cameraInfo;
    cameraInfo.offset = 0;
    cameraInfo.buffer = uniformBuffers[engine.renderer.priv_activeFrame % 2].buffer;
    cameraInfo.range = sizeof(PLCore_CameraUniform);
    WS_UniformB(&engine, "ubo", cameraInfo, VK_NULL_HANDLE);


    int loop = 0;
    bool debugRuntime = false;
    while (WindowInValidState(&engine)) {
        if (debugRuntime && loop >= 5) { break; }
        WS_StartFrame(&engine);




        WS_RenderGeometry(&engine);

        WS_EndFrame(&engine);
        loop++;
    }

    return 0;
}

