//
// Created by Owner on 11/20/2022.
//

#ifndef PLUTONIUM_ENGINE_HPP
#define PLUTONIUM_ENGINE_HPP

#define ENGINE_PREFIX(prefix, name) prefix##name
#include "PlutoniumCore.h"

typedef struct {
    PLCore_RenderInstance instance;
    PLCore_Window window;
    PLCore_Renderer renderer;
    PLCore_GraphicsPipeline pipeline;
    struct vertexObjects {
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



#endif //PLUTONIUM_ENGINE_HPP
