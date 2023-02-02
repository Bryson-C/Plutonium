//
// Created by Owner on 11/20/2022.
//

#ifndef PLUTONIUM_ENGINE_HPP
#define PLUTONIUM_ENGINE_HPP

#define ENGINE_PREFIX(prefix, name) prefix##name
#include "PlutoniumCore.h"
/*
typedef struct {
    PLCore_RenderInstance instance;
    PLCore_Window window;
    PLCore_Renderer renderer;
    PLCore_GraphicsPipeline pipeline;
    struct vertexObjects {
        bool needsUpdate;

        uint32_t msTillUpdate;
        clock_t updateTimer;

        uint32_t bufferMaxCount;
        uint32_t vertexCount;
        PLCore_Vertex* vertices;
        PLCore_Buffer vertexBuffer;
    } priv_vertObj;
    struct descriptorObjects {
        uint32_t descriptorCount;
        PLCore_DescriptorSet* descriptors;
        VkDescriptorSetLayout* layouts;
    } priv_descriptorObj;

} ENGINE_PREFIX(WS_, Engine);


WS_Engine       ENGINE_PREFIX(WS_, Initialize)(uint32_t width, uint32_t height);
WS_Engine       ENGINE_PREFIX(WS_, InitializeRenderer)(WS_Engine engine, PLCore_ShaderModule vertexShader, PLCore_ShaderModule fragmentShader);
bool            ENGINE_PREFIX(WS_, StartRenderLoop)(WS_Engine* engine);
void            ENGINE_PREFIX(WS_, StopRenderLoop)(WS_Engine* engine);
void            ENGINE_PREFIX(WS_, BindDescriptors)(WS_Engine* engine);

void            ENGINE_PREFIX(WS_, CreateVertexBuffer)(WS_Engine* engine);
PLCore_Vertex*  ENGINE_PREFIX(WS_, NewQuad)(WS_Engine* engine, float x, float y, float z, float w, float h, uint32_t id);

void            ENGINE_PREFIX(WS_, Draw)(WS_Engine* engine);
*/

typedef struct {
    enum ReturnStage {
        STAGE_DONT_CARE,
        STAGE_INITIALIZATION,
    } stage;
    enum ReturnValue {
        RETURN_SUCCESS,
        RETURN_FAILURE
    } value;
} ReturnResult;
inline ReturnResult SuccessfulReturn() { ReturnResult result = {.stage = STAGE_DONT_CARE, .value = RETURN_SUCCESS}; return result; }
inline ReturnResult SuccessfulReturnStage(enum ReturnStage stage) { ReturnResult result = {.stage = stage, .value = RETURN_SUCCESS}; return result; }
inline ReturnResult FailureReturn() { ReturnResult result = {.stage = STAGE_DONT_CARE, .value = RETURN_FAILURE}; return result; }
inline ReturnResult FailureReturnStage(enum ReturnStage stage) { ReturnResult result = {.stage = stage, .value = RETURN_FAILURE}; return result; }

typedef struct {
    uint32_t width;
    uint32_t height;
} EngineInfo;
typedef struct {
    PLCore_RenderInstance instance;
    PLCore_Window window;
    PLCore_Renderer renderer;

    PLCore_GraphicsPipeline pipeline;

    VkDescriptorPool descriptorPool;
    uint32_t descriptorSetCount;
    PLCore_DescriptorSet* descriptors;

    VkDescriptorSet* descriptorSets;
    VkDescriptorSetLayout* descriptorLayouts;
/*
    VkDescriptorSet* descriptorSets;
    VkDescriptorType* descriptorTypes;
    char** descriptorNames;
*/
} Engine;

ReturnResult ENGINE_PREFIX(WS_, Start)(Engine* engine, const char* vertexShaderPath, const char* fragmentShaderPath, EngineInfo* additionalInfo);
ReturnResult ENGINE_PREFIX(WS_, StartFrame)(Engine* engine);
ReturnResult ENGINE_PREFIX(WS_, EndFrame)(Engine* engine);
ReturnResult ENGINE_PREFIX(WS_, GetUniform)(Engine* engine, const char* name, PLCore_DescriptorSet* retDescriptor);
ReturnResult ENGINE_PREFIX(WS_, UniformW)(Engine* engine, const char* name, VkWriteDescriptorSet* writes, uint32_t writeCount);
ReturnResult ENGINE_PREFIX(WS_, UniformB)(Engine* engine, const char* name, VkDescriptorBufferInfo bufferInfo, PLCore_DescriptorAdditionalInfo* additionalInfo);
ReturnResult ENGINE_PREFIX(WS_, UniformI)(Engine* engine, const char* name, VkDescriptorImageInfo imageInfo, PLCore_DescriptorAdditionalInfo* additionalInfo);
ReturnResult ENGINE_PREFIX(WS_, NewGeometry)(Engine* engine, PLCore_Vertex* vertices, uint32_t vertexCount);
ReturnResult ENGINE_PREFIX(WS_, RenderGeometry)(Engine* engine);


typedef struct {
    PLCore_ShaderModule vertexShader;
    PLCore_ShaderModule fragmentShader;
} RendererModificationInfo;
ReturnResult ENGINE_PREFIX(WS_, ModifyRenderer)(Engine* engine);

inline bool WindowInValidState(const Engine* engine) {
    glfwPollEvents();
    return !glfwWindowShouldClose(engine->window.window);
}


#endif //PLUTONIUM_ENGINE_HPP
