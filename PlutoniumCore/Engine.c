//
// Created by Owner on 11/20/2022.
//

#include "Engine.h"
/*
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

    glfwPollEvents();
    PLCore_BeginFrameAdditionalInfo additionalInfo;
    additionalInfo.beginStage = BEGIN_FRAME_FROM_BEGINNING;
    //PLCore_BeginFrame((*engine).instance, &engine->renderer, &engine->pipeline, &engine->window, &additionalInfo);
    PLCore_BeginFrame((*engine).instance, &engine->renderer, &engine->pipeline, &engine->window, VK_NULL_HANDLE);
    VkCommandBuffer activeBuffer = PLCore_ActiveRenderBuffer(engine->renderer);

    vkCmdBindPipeline(activeBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, engine->pipeline.pipeline);


    if (engine->priv_vertObj.vertices != VK_NULL_HANDLE) {
        const VkDeviceSize vertexOffsets[] = {0};
        //(*engine).priv_vertObj.vertexBuffer = PLCore_CreateGPUBuffer(engine->instance, sizeof(PLCore_Vertex) * engine->priv_vertObj.vertexCount, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, engine->priv_vertObj.vertices);
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
    PLCore_EndFrame(engine->instance, &engine->renderer, &engine->pipeline, &engine->window, VK_NULL_HANDLE);

    if (engine->priv_vertObj.needsUpdate && clock() - engine->priv_vertObj.updateTimer >= engine->priv_vertObj.msTillUpdate
        && (*engine).priv_vertObj.vertices != VK_NULL_HANDLE && engine->priv_vertObj.vertexCount > 0) {

        VkFence fence = engine->renderer.priv_renderFences[(engine->renderer.priv_activeFrame > 0) ? 0 : 1];

        vkWaitForFences(engine->instance.pl_device.device, 1, &fence, VK_TRUE, UINT64_MAX);

        (*engine).priv_vertObj.updateTimer = clock();

        vkDestroyBuffer(engine->instance.pl_device.device, (*engine).priv_vertObj.vertexBuffer.buffer, VK_NULL_HANDLE);
        vkFreeMemory(engine->instance.pl_device.device, (*engine).priv_vertObj.vertexBuffer.memory, VK_NULL_HANDLE);

        PLCore_CreateGPUBuffer(engine->instance,
                               engine->priv_vertObj.vertexCount * sizeof(PLCore_Vertex),
                               VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                               engine->priv_vertObj.vertices);

        (*engine).priv_vertObj.needsUpdate = false;
    }

}

void ENGINE_PREFIX(WS_, CreateVertexBuffer)(WS_Engine* engine) {
    (*engine).priv_vertObj.bufferMaxCount = 100;
    (*engine).priv_vertObj.vertexCount = 0;
    (*engine).priv_vertObj.vertices = malloc(sizeof(PLCore_Vertex) * engine->priv_vertObj.bufferMaxCount);
    (*engine).priv_vertObj.msTillUpdate = 16;
    (*engine).priv_vertObj.updateTimer = clock();
    (*engine).priv_vertObj.needsUpdate = false;
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

    (*engine).priv_vertObj.needsUpdate = true;

    return pVertLoc;
}

void ENGINE_PREFIX(WS_, Draw)(WS_Engine* engine) {
    vkCmdDraw(PLCore_ActiveRenderBuffer(engine->renderer), engine->priv_vertObj.vertexCount, 1, 0, 0);
}
*/

ReturnResult ENGINE_PREFIX(WS_, Start)(Engine* engine,
                                        const char* vertexShaderPath,
                                        const char* fragmentShaderPath,
                                        EngineInfo* additionalInfo) {
    if (engine == VK_NULL_HANDLE)
        return FailureReturnStage(STAGE_INITIALIZATION);

    (*engine).instance = PLCore_CreateRenderingInstance();
    (*engine).window = PLCore_CreateWindow(
            engine->instance,
            (additionalInfo != VK_NULL_HANDLE) ? additionalInfo->width : 800,
            (additionalInfo != VK_NULL_HANDLE) ? additionalInfo->height : 600);
    (*engine).renderer = PLCore_CreateRenderer(engine->instance, engine->window);
    (*engine).descriptorPool = PLCore_CreateGeneralizedDescriptorPool(engine->instance);

    PLCore_ShaderModule vShader = PLCore_CreateShader(engine->instance, vertexShaderPath, VK_SHADER_STAGE_VERTEX_BIT, "main", engine->descriptorPool);
    PLCore_ShaderModule fShader = PLCore_CreateShader(engine->instance, fragmentShaderPath, VK_SHADER_STAGE_FRAGMENT_BIT, "main", engine->descriptorPool);

    // Get All Descriptors From Shaders And Copy Them To Engine Struct
    (*engine).descriptorSetCount = vShader.descriptorSetCount + fShader.descriptorSetCount;
    (*engine).descriptorLayouts = malloc(sizeof(VkDescriptorSetLayout) * engine->descriptorSetCount);
    (*engine).descriptorSets = malloc(sizeof(PLCore_DescriptorSet) * engine->descriptorSetCount);
    //(*engine).descriptorTypes = malloc(sizeof(PLCore_DescriptorSet) * engine->descriptorSetCount);

    (*engine).descriptors = malloc(sizeof(PLCore_DescriptorSet) * engine->descriptorSetCount);

    //(*engine).descriptorNames = (char**)malloc(sizeof(char*) * engine->descriptorSetCount);
    static const uint16_t MAX_DESCRIPTOR_NAME_SIZE = 32;



    //for (int i = 0; i < engine->descriptorSetCount; i++)
        //(*engine).descriptorNames[i] = (char*)malloc(sizeof(char) * MAX_DESCRIPTOR_NAME_SIZE);


    for (int i = 0, descriptorIndex = 0; i < engine->descriptorSetCount; i++) {
        if (i < vShader.descriptorSetCount) {
            (*engine).descriptors[i] = vShader.descriptorSets[descriptorIndex];

            (*engine).descriptorSets[i] = vShader.descriptorSets[descriptorIndex].set;
            (*engine).descriptorLayouts[i] = vShader.descriptorSets[descriptorIndex].layout;
            //(*engine).descriptorTypes[i] = vShader.descriptorSets[descriptorIndex].type;
            //strcpy((*engine).descriptorNames[i], vShader.descriptorSets[descriptorIndex].name);

            (descriptorIndex >= vShader.descriptorSetCount-1) ? descriptorIndex = 0 : descriptorIndex++;
        } else {
            (*engine).descriptors[i] = fShader.descriptorSets[descriptorIndex];

            (*engine).descriptorLayouts[i] = fShader.descriptorSets[descriptorIndex].layout;
            (*engine).descriptorSets[i] = fShader.descriptorSets[descriptorIndex].set;
            //(*engine).descriptorTypes[i] = fShader.descriptorSets[descriptorIndex].type;
            //strcpy((*engine).descriptorNames[i], fShader.descriptorSets[descriptorIndex].name);

            (descriptorIndex >= fShader.descriptorSetCount-1) ? descriptorIndex = 0 : descriptorIndex++;
        }
    }

    for (int i = 0; i < engine->descriptorSetCount; i++) {
        printf("Shader Variable: %s\n", engine->descriptors[i].name);
    }

    VkPipelineLayout pipelineLayout = PLCore_Priv_CreatePipelineLayout(
            engine->instance.pl_device.device,
            0,
            VK_NULL_HANDLE,
            engine->descriptorSetCount,
            engine->descriptorLayouts);

    VkPipelineVertexInputStateCreateInfo vertexInputs = PLCore_CreateInputInfoFromShader(engine->instance, vShader);

    (*engine).pipeline = PLCore_CreatePipeline(engine->instance, engine->renderer, vertexInputs, vShader, fShader, &pipelineLayout);

    return SuccessfulReturnStage(STAGE_INITIALIZATION);
}

ReturnResult ENGINE_PREFIX(WS_, ModifyRenderer)(Engine* engine) {
    return FailureReturn();
}
ReturnResult ENGINE_PREFIX(WS_, StartFrame)(Engine* engine) {
    PLCore_BeginFrame(engine->instance, &engine->renderer, &engine->pipeline, &engine->window, VK_NULL_HANDLE);

    VkCommandBuffer activeBuffer = PLCore_ActiveRenderBuffer(engine->renderer);

    vkCmdBindPipeline(activeBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, engine->pipeline.pipeline);
    vkCmdBindDescriptorSets(activeBuffer,
                            VK_PIPELINE_BIND_POINT_GRAPHICS,
                            engine->pipeline.layout,
                            0,
                            engine->descriptorSetCount,
                            engine->descriptorSets,
                            0,
                            VK_NULL_HANDLE);

    return SuccessfulReturn();
}
ReturnResult ENGINE_PREFIX(WS_, EndFrame)(Engine* engine) {
    PLCore_EndFrame(engine->instance, &engine->renderer, &engine->pipeline, &engine->window, VK_NULL_HANDLE);
    return SuccessfulReturn();
}
static ReturnResult GetUniformByName(Engine* engine, const char* name, uint32_t* retOffset) {

    // go through engine descriptors name list
    // find `name` in list
    // set value of found descriptor to `value`

    for (int i = 0; i < engine->descriptorSetCount; i++) {
        if (strcmp(engine->descriptors[i].name, name) == 0) {
            *retOffset = i;
            return SuccessfulReturn();
        }
    }
    return FailureReturn();

}
ReturnResult ENGINE_PREFIX(WS_, GetUniform)(Engine* engine, const char* name, PLCore_DescriptorSet* retDescriptor) {
    uint32_t descriptorOffset = 0;
    ReturnResult result = GetUniformByName(engine, name, &descriptorOffset);
    *retDescriptor = engine->descriptors[descriptorOffset];
    return result;
}
ReturnResult ENGINE_PREFIX(WS_, UniformW)(Engine* engine, const char* name, VkWriteDescriptorSet* writes, uint32_t writeCount) {
    uint32_t descriptorOffset = 0;
    ReturnResult result = GetUniformByName(engine, name, &descriptorOffset);

    vkUpdateDescriptorSets(engine->instance.pl_device.device, writeCount, writes, 0, VK_NULL_HANDLE);

    return result;
}
ReturnResult ENGINE_PREFIX(WS_,UniformB)(Engine* engine, const char* name, VkDescriptorBufferInfo bufferInfo, PLCore_DescriptorAdditionalInfo* additionalInfo) {
    uint32_t descriptorOffset = 0;
    ReturnResult result = GetUniformByName(engine, name, &descriptorOffset);
    PLCore_UpdateDescriptor(engine->instance,
                            engine->descriptors[descriptorOffset].set,
                            engine->descriptors[descriptorOffset].type,
                            engine->descriptors[descriptorOffset].slot,
                            &bufferInfo, VK_NULL_HANDLE, additionalInfo);
    return result;
}
ReturnResult ENGINE_PREFIX(WS_,UniformI)(Engine* engine, const char* name, VkDescriptorImageInfo imageInfo, PLCore_DescriptorAdditionalInfo* additionalInfo) {
    uint32_t descriptorOffset = 0;
    ReturnResult result = GetUniformByName(engine, name, &descriptorOffset);
    PLCore_UpdateDescriptor(engine->instance,
                            engine->descriptors[descriptorOffset].set,
                            engine->descriptors[descriptorOffset].type,
                            engine->descriptors[descriptorOffset].slot,
                            VK_NULL_HANDLE, &imageInfo, additionalInfo);
    return result;
}
static PLCore_Buffer TEST_engineVertexBuffer;
static bool TEST_rebuildEngineVertexBuffer = true;
static uint32_t TEST_vertexCount = 0;
ReturnResult ENGINE_PREFIX(WS_, NewGeometry)(Engine* engine, PLCore_Vertex* vertices, uint32_t vertexCount) {
    if (TEST_rebuildEngineVertexBuffer) {
        TEST_engineVertexBuffer = PLCore_CreateGPUBuffer(engine->instance,
                                                         sizeof(PLCore_Vertex) * vertexCount,
                                                         VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                                                         vertices);
        TEST_vertexCount = vertexCount;
        TEST_rebuildEngineVertexBuffer = false;
    }
    return SuccessfulReturn();
}
ReturnResult ENGINE_PREFIX(WS_, RenderGeometry)(Engine* engine) {

    VkCommandBuffer activeBuffer = PLCore_ActiveRenderBuffer(engine->renderer);
    VkDeviceSize offsets[1] = { 0 };
    vkCmdBindVertexBuffers(activeBuffer, 0, 1, &TEST_engineVertexBuffer.buffer, offsets);

    // TODO: Fix This For Max Efficiency

    vkCmdDraw(activeBuffer, TEST_vertexCount, 1, 0, 0);

    return SuccessfulReturn();
}