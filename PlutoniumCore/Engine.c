//
// Created by Owner on 11/20/2022.
//

#include "Engine.h"

WS_Engine ENGINE_PREFIX(WS_, Initialize)(uint32_t width, uint32_t height) {
    WS_Engine Engine;
    Engine.instance = PLCore_CreateRenderingInstance();
    Engine.window = PLCore_CreateWindow(Engine.instance, width, height);
    Engine.renderer = PLCore_CreateRenderer(Engine.instance, Engine.window);
    WS_CreateVertexBuffer(&Engine);
    return Engine;
}
WS_Engine ENGINE_PREFIX(WS_, InitializeRenderer)(WS_Engine engine, PLCore_ShaderModule vertexShader, PLCore_ShaderModule fragmentShader) {

    VkPipelineVertexInputStateCreateInfo vertexInput = PLCore_CreateInputInfoFromShader(engine.instance, vertexShader);

    uint32_t descriptorCount =
            engine.priv_descriptorObj.descriptorCount =
            vertexShader.descriptorSetCount + fragmentShader.descriptorSetCount;

    engine.priv_descriptorObj.descriptors = malloc(sizeof(PLCore_DescriptorSet) * descriptorCount);
    engine.priv_descriptorObj.layouts = malloc(sizeof(VkDescriptorSetLayout) * descriptorCount);

    int descriptorArrayOffset = 0;
    for (int i = 0; i < vertexShader.descriptorSetCount; i++, descriptorArrayOffset++) {
        engine.priv_descriptorObj.descriptors[descriptorArrayOffset] = vertexShader.descriptorSets[i];
        engine.priv_descriptorObj.layouts[descriptorArrayOffset] = vertexShader.descriptorSets[i].layout;
    }

    for (int i = 0; i < fragmentShader.descriptorSetCount; i++, descriptorArrayOffset++) {
        engine.priv_descriptorObj.descriptors[descriptorArrayOffset] = fragmentShader.descriptorSets[i];
        engine.priv_descriptorObj.layouts[descriptorArrayOffset] = fragmentShader.descriptorSets[i].layout;
    }

    engine.pipeline.layout = PLCore_Priv_CreatePipelineLayout(engine.instance.pl_device.device, 0, VK_NULL_HANDLE, descriptorCount, engine.priv_descriptorObj.layouts);
    engine.pipeline = PLCore_CreatePipeline(engine.instance, engine.renderer, vertexInput, vertexShader, fragmentShader, &engine.pipeline.layout);

    return engine;
}

bool ENGINE_PREFIX(WS_, StartRenderLoop)(WS_Engine* engine) {
    // update vertex buffer if needed
    // TODO: This Crashes Code
    /*
    if (clock() - engine->priv_vertObj.updateTimer >= engine->priv_vertObj.msTillUpdate && (*engine).priv_vertObj.vertices != VK_NULL_HANDLE) {
        (*engine).priv_vertObj.updateTimer = clock();
        VkCommandBuffer updateBuffer = PLCore_Priv_CreateCommandBuffers(engine->instance.pl_device.device,
                                                                        engine->renderer.graphicsPool.pool, 1)[0];
        PLCore_RecordCommandBuffer(updateBuffer);

        vkCmdUpdateBuffer(updateBuffer,
                          engine->priv_vertObj.vertexBuffer.buffer,
                          sizeof(PLCore_Vertex) * engine->priv_vertObj.vertexCount,
                          sizeof(PLCore_Vertex) * engine->priv_vertObj.vertexCount,
                          engine->priv_vertObj.vertices);

        PLCore_StopCommandBuffer(updateBuffer);
        PLCore_SubmitCommandBuffer(updateBuffer, engine->instance.pl_device.graphicsQueue.queue, VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE);
    }*/

    glfwPollEvents();
    PLCore_BeginFrame((*engine).instance, &engine->renderer, &engine->pipeline, &engine->window);

    VkCommandBuffer activeBuffer = PLCore_ActiveRenderBuffer(engine->renderer);
    vkCmdBindPipeline(activeBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, engine->pipeline.pipeline);

    if (engine->priv_vertObj.vertices != VK_NULL_HANDLE) {
        const VkDeviceSize vertexOffsets[] = {0};
        //engine->priv_vertObj.vertexBuffer = PLCore_CreateGPUBuffer(engine->instance, sizeof(PLCore_Vertex) * engine->priv_vertObj.vertexCount, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, engine->priv_vertObj.vertices);
        vkCmdBindVertexBuffers(activeBuffer, 0, 1, &engine->priv_vertObj.vertexBuffer.buffer, vertexOffsets);
    }


    return !glfwWindowShouldClose(engine->window.window);
}
void ENGINE_PREFIX(WS_, BindDescriptors)(WS_Engine* engine) {
    VkCommandBuffer activeBuffer = PLCore_ActiveRenderBuffer(engine->renderer);

    VkDescriptorSet bindSets[32];
    for (int i = 0; i < engine->priv_descriptorObj.descriptorCount; i++) {
        if (i >= 32) { fprintf(stderr, "Cannot Bind More Than 32 Descriptors Sets, Bound: %i\n", i); break; }
        bindSets[i] = engine->priv_descriptorObj.descriptors[i].set;
    }

    vkCmdBindDescriptorSets(activeBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            engine->pipeline.layout, 0,
                            engine->priv_descriptorObj.descriptorCount,
                            bindSets, 0, VK_NULL_HANDLE);

}
void ENGINE_PREFIX(WS_, StopRenderLoop)(WS_Engine* engine) {
    PLCore_EndFrame(engine->instance, &engine->renderer, &engine->pipeline, &engine->window);
}

void ENGINE_PREFIX(WS_, CreateVertexBuffer)(WS_Engine* engine) {
    (*engine).priv_vertObj.bufferMaxCount = 100;
    (*engine).priv_vertObj.vertexCount = 0;
    (*engine).priv_vertObj.vertices = malloc(sizeof(PLCore_Vertex) * engine->priv_vertObj.bufferMaxCount);
    (*engine).priv_vertObj.msTillUpdate = 16;
    (*engine).priv_vertObj.updateTimer = clock();
    (*engine).priv_vertObj.vertexBuffer = PLCore_CreateGPUBuffer(engine->instance,
                                                               sizeof(PLCore_Vertex) * engine->priv_vertObj.bufferMaxCount,
                                                               VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                                                               engine->priv_vertObj.vertices);
}


PLCore_Vertex* ENGINE_PREFIX(WS_, NewQuad)(WS_Engine* engine, float x, float y, float z, float w, float h, uint32_t id) {
    PLCore_Vertex vertices[] = {
        {{ x, y, z },          { 0.0f, 0.0f, 0.0f }, { -1.0f, 0.0f }, id },
        {{ x + w, y, z },      { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f },  id },
        {{ x + w, y + h, z },  { 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f },  id },
        {{ x, y + h, z },      { 0.0f, 0.0f, 0.0f }, { -1.0f, 1.0f }, id }
    };
    PLCore_Vertex* pVertLoc = &engine->priv_vertObj.vertices[engine->priv_vertObj.vertexCount];
    (*engine).priv_vertObj.vertices[engine->priv_vertObj.vertexCount] = vertices[0];
    (*engine).priv_vertObj.vertices[engine->priv_vertObj.vertexCount+1] = vertices[1];
    (*engine).priv_vertObj.vertices[engine->priv_vertObj.vertexCount+2] = vertices[2];
    (*engine).priv_vertObj.vertices[engine->priv_vertObj.vertexCount+3] = vertices[3];
    (*engine).priv_vertObj.vertexCount += 4;

    return pVertLoc;
}