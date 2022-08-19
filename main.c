#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

/*
//#include "Abstraction/Abstractions.h"
#include "RenderState/RenderState.h"


#define STB_IMAGE_IMPLEMENTATION 1
#include "lib/stb_image.h"
#include "vulkan/vulkan.h"
#include "shaderc.h"



typedef uint32_t U32;

static Buffer s_IndexBuffer;
static U32* s_GlobalIndices = VK_NULL_HANDLE;
static size_t s_GlobalIndexCount = 0;
static size_t s_GlobalIndexSize = 100;
static U32 s_GlobalIndexOffset = 0;
static bool s_GlobalIndicesChanged = false;

static Buffer s_VertexBuffer;
static vertex* s_GlobalVertices = VK_NULL_HANDLE;
static size_t s_GlobalVertexCount = 0;
static size_t s_GlobalVertexSize = 100;
static bool s_GlobalVerticesChanged = false;

vertex* CreateNewQuadViaVertices(float x, float y, float w, float h, Float3 color, Float textureIndex) {
    if (s_GlobalVertices == VK_NULL_HANDLE) s_GlobalVertices = malloc(sizeof(vertex) * s_GlobalVertexSize);
    if (s_GlobalVertexCount >= s_GlobalVertexSize - 10) s_GlobalVertices = realloc(s_GlobalVertices, sizeof(vertex) * (s_GlobalVertexSize *= 2));

    if (s_GlobalIndices == VK_NULL_HANDLE) s_GlobalIndices =  malloc(sizeof(U32) * s_GlobalIndexSize);
    if (s_GlobalIndexCount >= s_GlobalIndexSize - 10) s_GlobalIndices = realloc(s_GlobalIndices, sizeof(U32) * (s_GlobalIndexSize *= 2));

    vertex vertices[4] = {
        {{x,y,0.0f},color, {1.0f, 0.0f}, textureIndex},
        {{x + w,y,0.0f},color,{0.0f, 0.0f}, textureIndex},
        {{x + w,y + h,0.0f},color,{0.0f, 1.0f}, textureIndex},
        {{x,y + h,0.0f},color,{1.0f, 1.0f}, textureIndex},
    };
    memcpy(s_GlobalVertices+s_GlobalVertexCount, vertices, sizeof(vertex) * 4);
    s_GlobalVertexCount += 4;


    U32 indexCount = 6;
    U32 indices[] = {s_GlobalIndexOffset, s_GlobalIndexOffset+1, s_GlobalIndexOffset+2, s_GlobalIndexOffset+2, s_GlobalIndexOffset+3, s_GlobalIndexOffset};
    s_GlobalIndexOffset += 4;
    memcpy(s_GlobalIndices+s_GlobalIndexCount, indices, sizeof(U32) * indexCount);
    s_GlobalIndexCount += indexCount;

    printf("Vertices: %zi/%zi\n", s_GlobalVertexCount, s_GlobalVertexSize);
    printf("Indices: %zi/%zi\n", s_GlobalIndexCount, s_GlobalIndexSize);
    s_GlobalVerticesChanged = true;
    s_GlobalIndicesChanged = true;
    return vertices;
}

vertex* AcquireVertices() { return s_GlobalVertices; }
U32* AcquireIndices(size_t* indexCount) { *indexCount = s_GlobalIndexCount; return s_GlobalIndices; }
Buffer RequestBufferFromGlobalVertices()
{
    s_GlobalVerticesChanged = false;
    return RequestBufferToGPU(sizeof(vertex) * s_GlobalVertexCount, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, s_GlobalVertices);
}
Buffer RequestBufferFromGlobalIndices()
{
    s_GlobalIndicesChanged = false;
    return RequestBufferToGPU(sizeof(U32) * s_GlobalIndexCount, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, s_GlobalIndices);
}
*/



#include "Abstraction/PlutoniumCore/PlutoniumCore.h"

#include "Abstraction/PlutoniumCore/WrenScript.h"



int main() {
/*
    const Float3 softBlue = {0.1f,0.3f,1.0f};
    const Float3 softRed = {1.0f,0.3f,0.1f};
    const Float3 softGreen = {0.1f,1.0f,0.3f};
    const Float3 white = {0.9f,0.9f,0.9f};

    CreateNewQuadViaVertices(-1.0f, -1.0f, 1.0f, 1.0f, white, -1);
    CreateNewQuadViaVertices(0.0f, 0.0f, 1.0f, 1.0f, white, -1);


    ApplyShader(VK_SHADER_STAGE_VERTEX_BIT, "D:\\Plutonium\\vertex.spv", "main");
    ApplyShader(VK_SHADER_STAGE_FRAGMENT_BIT, "D:\\Plutonium\\fragment.spv", "main");

    U32 bindingCount = 1;
    VkVertexInputBindingDescription bindings[] = {
            createVertexBinding(0, VK_VERTEX_INPUT_RATE_VERTEX, sizeof(vertex))
    };

    U32 attributeCount = 3;
    VkVertexInputAttributeDescription attributes[] = {
            createVertexAttribute(0, 0, (VkFormat)shaderVec3, offsetof(vertex,pos)),
            createVertexAttribute(0, 1, (VkFormat)shaderVec3, offsetof(vertex,col)),
            createVertexAttribute(0, 2, (VkFormat)shaderVec2, offsetof(vertex,uv)),
            createVertexAttribute(0, 3, (VkFormat)shaderFloat, offsetof(vertex,textureId)),
    };
    ApplyVertexInput(attributeCount, attributes, bindingCount, bindings);

    CreateRenderState();


    Buffer VertexBuffer = RequestBufferFromGlobalVertices();
    Buffer IndexBuffer = RequestBufferFromGlobalIndices();

    UniformBuffer UniformBuffers[2] = {
        RequestUniform(sizeof(UBO), VK_SHADER_STAGE_FRAGMENT_BIT, 0),
        RequestUniform(sizeof(UBO), VK_SHADER_STAGE_FRAGMENT_BIT, 0),
    };

    Texture* texture = RequestTexture("D:\\Plutonium\\texture.jpg", 0);
    //Texture* texture2 = RequestTexture("D:\\Plutonium\\circ.png", 1);
    //Texture* texture3 = RequestTexture("D:\\Plutonium\\texture.jpg", 2);

    VkDescriptorSetLayout layouts[] = {UniformBuffers[0].descriptor.layout, VRS.texturePool.layout[0], VRS.texturePool.samplerLayout};
    ApplyDescriptorLayouts(3, layouts);

    CreateRenderStateRenderer();

    clock_t timer = clock();

    while (!glfwWindowShouldClose(VRS.window)) {
        glfwPollEvents();
        BeginDraw();


        float color = sin((double)clock()/1000)/1.0f + 0.5f;
        UBO ubo = {.xyz = softGreen};

        UpdateUniform(VRS.device, &UniformBuffers[VRS.activeFrame], sizeof(UBO), &ubo);

        CommandBuffer currentBuffer = VRS.renderBuffers[VRS.imageIndex];
        VkDeviceSize offsets[] = {0};

        VkDescriptorSet sets[] = {UniformBuffers[VRS.activeFrame].descriptor.set, VRS.texturePool.samplerSet, texture->set};

        BindDescriptorSets(currentBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, VRS.pipelineLayout, 3, sets);

        vkCmdBindVertexBuffers(currentBuffer, 0, 1, &VertexBuffer.buffer, offsets);
        vkCmdBindIndexBuffer(currentBuffer, IndexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);

        size_t indexCount;
        AcquireIndices(&indexCount);
        vkCmdDrawIndexed(currentBuffer, indexCount, 1, 0, 0, 0);

        SubmitDraw();
        glfwSwapBuffers(VRS.window);

        if (vkGetFenceStatus(VRS.device, VRS.renderFences[VRS.activeFrame]) == VK_NOT_READY) {
            if (s_GlobalVerticesChanged) VertexBuffer = RequestBufferFromGlobalVertices();
            if (s_GlobalIndicesChanged) IndexBuffer = RequestBufferFromGlobalIndices();
        }

    }


    DestroyRenderState();
*/

    // remove If You Want The Rendering Code
    //return wrenMain("D:\\Plutonium\\wren.wren");


    PLCore_RenderInstance RenderInstance = PLCore_CreateRenderingInstance();
    PLCore_Window Window = PLCore_CreateWindow(RenderInstance.pl_instance.instance, 800, 600);
    PLCore_Renderer Renderer = PLCore_CreateRenderer(RenderInstance, Window);


    PLCore_ShaderModule vShader = PLCore_Priv_CreateShader(RenderInstance.pl_device.device, "D:\\Plutonium\\Shaders\\v.spv", "main");
    PLCore_ShaderModule fShader = PLCore_Priv_CreateShader(RenderInstance.pl_device.device, "D:\\Plutonium\\Shaders\\f.spv", "main");
    s_GlobalVertexShader = vShader;
    s_GlobalFragmentShader = fShader;


    // TODO: Vertices Are Not Correctly Placed At The Right Coordinants
    PLCore_vertex vertices[] = {
        {{ 0.0f, -0.5f, 0.0f}, {1.0f, 1.0f,  1.0f}},
        {{ 0.5f, 0.5f, 0.0f }, { 0.0f, 1.0f, 0.0f }},
        {{ -0.5f, 0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f }},

        {{0.0f-1.0f, -0.5f, 1.0f}, { 1.0f, 0.0f, 0.0f }},
        {{ 0.5f-1.0f, 0.5f, 1.0f }, { 1.0f, 0.0f, 0.0f }},
        {{ -0.5f-1.0f, 0.5f, 1.0f }, { 1.0f, 0.0f, 0.0f }},
    };

    PLCore_vertex verts2[] = {
            {{ 0.0f+1.0f, -0.5f, 1.0f}, { 1.0f, 0.0f, 0.0f }},
            {{ 0.5f+1.0f, 0.5f, 1.0f }, { 1.0f, 0.0f, 0.0f }},
            {{ -0.5f+1.0f, 0.5f, 1.0f }, { 1.0f, 0.0f, 0.0f }},
    };

    PLCore_DynamicSizedBuffer dynVertexBuffer = PLCore_CreateDynamicSizedBuffer();
    PLCore_PushVerticesToDynamicSizedBuffer(&dynVertexBuffer, sizeof(PLCore_vertex), 6, vertices);
    PLCore_PushVerticesToDynamicSizedBuffer(&dynVertexBuffer, sizeof(PLCore_vertex), 3, verts2);
    PLCore_Buffer vertexBuffer = PLCore_RequestDynamicSizedBufferToGPU(RenderInstance, &dynVertexBuffer, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, sizeof(PLCore_vertex));




    VkVertexInputAttributeDescription attribs[] = {
            {
                .binding = 0,
                .format = VK_FORMAT_R32G32B32_SFLOAT,
                .offset = offsetof(PLCore_vertex, xyz),
                .location = 0,
            },
            {
                .binding = 0,
                .format = VK_FORMAT_R32G32B32_SFLOAT,
                .offset = offsetof(PLCore_vertex, rgb),
                .location = 1,
            }
    };
    VkVertexInputBindingDescription bindings[] = {
            {
                .binding = 0,
                .stride = sizeof(PLCore_vertex),
                .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
            }
    };
    VkPipelineVertexInputStateCreateInfo vertexInput = PLCore_Priv_CreateVertexInput(2, attribs, 1, bindings);

    PLCore_GraphicsPipeline Pipeline = PLCore_CreatePipeline(RenderInstance, Renderer, Window, vertexInput, vShader, fShader);

    uint32_t fps = 0;
    clock_t timer = clock();
    clock_t buttonCooldown = clock();


    while(!glfwWindowShouldClose(Window.window)) {
        // If The Vertex Data Has Been Updated
        if (glfwGetMouseButton(Window.window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS && clock() - buttonCooldown > 200) {
            buttonCooldown = clock();
            double xMouse;
            glfwGetCursorPos(Window.window, &xMouse, VK_NULL_HANDLE);
            srand((size_t)xMouse + clock());

            float xOffset = ((float)(rand() % 200)/200.0f)-0.5f, yOffset = ((float)(rand() % 200)/200.0f)-0.5f;
            float
            colx = (float)sin(clock()%100),
            coly = (float)sin(clock()%50),
            colz = (float)sin(clock()%30);

            PLCore_vertex verts[] = {
                    {{ 0.0f+xOffset, -0.5f+yOffset, 0.0f}, { colx, 1.0f,  1.0f }},
                    {{ 0.5f+xOffset, 0.5f+yOffset, 0.0f }, { 0.0f, coly, 0.0f }},
                    {{ -0.5f+xOffset, 0.5f+yOffset, 0.0f }, { 0.0f, 0.0f, colz }},
            };
            PLCore_PushVerticesToDynamicSizedBuffer(&dynVertexBuffer, sizeof(PLCore_vertex), 3, verts);
        } else if (glfwGetMouseButton(Window.window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS && clock() - buttonCooldown > 200) {
            PLCore_ClearDynamicSizedBufferData(&dynVertexBuffer);
        }
        if (dynVertexBuffer.dataChanged == 1) {
            vertexBuffer = PLCore_RequestDynamicSizedBufferToGPU(RenderInstance, &dynVertexBuffer, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, sizeof(PLCore_vertex));
        }

        glfwPollEvents();

        PLCore_BeginFrame(RenderInstance, &Renderer, &Pipeline, &Window);

        VkCommandBuffer activeBuffer = PLCore_ActiveRenderBuffer(Renderer);

        vkCmdBindPipeline(activeBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, Pipeline.pipeline);
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(activeBuffer, 0, 1, &vertexBuffer.buffer, offsets);

        vkCmdDraw(activeBuffer, dynVertexBuffer.dataCount, 1, 0, 0);


        PLCore_EndFrame(RenderInstance, &Renderer, &Pipeline, &Window);
        glfwSwapBuffers(Window.window);

        fps++;
        if (clock()-timer > 1000) {
            printf("Drawing %zi Vertices\n", dynVertexBuffer.dataCount);
            timer = clock();
            printf("FPS: %u\n", fps);
            fps = 0;
        }
    }


    return 0;
}
