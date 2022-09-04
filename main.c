#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#include <windows.h>

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

typedef struct {
    float x;
    float y;
} UNIFORM;


int main() {

    // remove If You Want The Rendering Code

    int ScriptThreadSafelyClosed;
    HANDLE ScriptHandle = NewWrenScriptThread("D:\\Plutonium\\wren.wren");

    PLCore_RenderInstance RenderInstance = PLCore_CreateRenderingInstance();
    PLCore_Window Window = PLCore_CreateWindow(RenderInstance.pl_instance.instance, 800, 600);
    PLCore_Renderer Renderer = PLCore_CreateRenderer(RenderInstance, Window);


    PLCore_ShaderModule vShader = PLCore_Priv_CreateShader(RenderInstance.pl_device.device, "D:\\Plutonium\\Shaders\\v.spv", "main");
    PLCore_ShaderModule fShader = PLCore_Priv_CreateShader(RenderInstance.pl_device.device, "D:\\Plutonium\\Shaders\\f.spv", "main");



    // TODO: Vertices Are Not Correctly Placed At The Right Coordinants
    PLCore_Vertex vertices[] = {
            {{ 0.0f, -0.5f, 0.0f}, {1.0f, 1.0f,  1.0f}},
            {{ 0.5f, 0.5f, 0.0f }, { 0.0f, 1.0f, 0.0f }},
            {{ -0.5f, 0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f }},

            {{0.0f-1.0f, -0.5f, 1.0f}, { 1.0f, 0.0f, 0.0f }},
            {{ 0.5f-1.0f, 0.5f, 1.0f }, { 1.0f, 0.0f, 0.0f }},
            {{ -0.5f-1.0f, 0.5f, 1.0f }, { 1.0f, 0.0f, 0.0f }},
    };

    PLCore_Vertex verts2[] = {
            {{ 0.0f+1.0f, -0.5f, 1.0f}, { 1.0f, 0.0f, 0.0f }},
            {{ 0.5f+1.0f, 0.5f, 1.0f }, { 1.0f, 0.0f, 0.0f }},
            {{ -0.5f+1.0f, 0.5f, 1.0f }, { 1.0f, 0.0f, 0.0f }},
    };

    PLCore_DynamicVertexBuffer dynVertexBuffer = PLCore_CreateDynamicVertexBuffer();
    PLCore_PushVerticesToDynamicVertexBuffer(&dynVertexBuffer, sizeof(PLCore_Vertex), 6, vertices);
    PLCore_Vertex* vert = PLCore_PushVerticesToDynamicVertexBuffer(&dynVertexBuffer, sizeof(PLCore_Vertex), 3, verts2);
    PLCore_Buffer vertexBuffer = PLCore_RequestDynamicVertexBufferToGPU(RenderInstance, &dynVertexBuffer, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, sizeof(PLCore_Vertex));



    PLCore_Buffer uniformBuffers[2] = {
            PLCore_CreateBuffer(RenderInstance, sizeof(UNIFORM), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT),
            PLCore_CreateBuffer(RenderInstance, sizeof(UNIFORM), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT)
    };
    VkDescriptorSetLayoutBinding binding = PLCore_Priv_CreateDescriptorLayoutBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT);
    VkDescriptorSetLayout layout = PLCore_Priv_CreateDescriptorLayout(RenderInstance.pl_device.device, 1, &binding);
    VkDescriptorPoolSize size = PLCore_Priv_CreateDescritorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, Renderer.backBuffers);
    VkDescriptorPool pool = PLCore_Priv_CreateDescriptorPool(RenderInstance.pl_device.device, 2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, &size);
    VkDescriptorSet* uniformSets = PLCore_Priv_CreateDescriptorSets(RenderInstance.pl_device.device, 2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, layout, pool);

    VkDescriptorBufferInfo bufferinfo[2] = {
            {
                .buffer = uniformBuffers[0].buffer,
                .offset = 0,
                .range = sizeof(UNIFORM),
            },{
                .buffer = uniformBuffers[1].buffer,
                .offset = 0,
                .range = sizeof(UNIFORM),
            },
    };
    PLCore_Priv_WriteDescriptor(RenderInstance.pl_device.device, uniformSets[0], VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, &bufferinfo[0], VK_NULL_HANDLE);
    PLCore_Priv_WriteDescriptor(RenderInstance.pl_device.device, uniformSets[1], VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, &bufferinfo[1], VK_NULL_HANDLE);


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
            }
    };
    VkVertexInputBindingDescription bindings[] = {
            {
                .binding = 0,
                .stride = sizeof(PLCore_Vertex),
                .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
            }
    };
    VkPipelineVertexInputStateCreateInfo vertexInput = PLCore_Priv_CreateVertexInput(2, attribs, 1, bindings);

    VkPipelineLayout pipelineLayout = PLCore_Priv_CreatePipelineLayout(RenderInstance.pl_device.device, 0, VK_NULL_HANDLE, 1, &layout);

    PLCore_GraphicsPipeline Pipeline = PLCore_CreatePipeline(RenderInstance, Renderer, vertexInput, vShader, fShader, &pipelineLayout);

    uint32_t fps = 0;
    clock_t timer = clock();
    clock_t buttonCooldown = clock();

    float xPos = 0.0f, yPos = 0.0f;
    clock_t moveTimerX = clock(), moveTimerY = clock();

    while(!glfwWindowShouldClose(Window.window)) {
        // If The Vertex Data Has Been Updated
        if (glfwGetMouseButton(Window.window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS && clock() - buttonCooldown > 200) {
            buttonCooldown = clock();
            double xMouse, yMouse;
            glfwGetCursorPos(Window.window, &xMouse, &yMouse);

            PLCore_MoveDynamicBufferVertices(&dynVertexBuffer, vert, 3, -0.01f, -0.01f);

        } else if (glfwGetMouseButton(Window.window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS && clock() - buttonCooldown > 200) {
            PLCore_ClearDynamicVertexBufferData(&dynVertexBuffer);
        }
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

        vkCmdBindDescriptorSets(activeBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, Pipeline.layout, 0, 1, &uniformSets[Renderer.priv_activeFrame], 0, VK_NULL_HANDLE);

        vkCmdBindPipeline(activeBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, Pipeline.pipeline);
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(activeBuffer, 0, 1, &vertexBuffer.buffer, offsets);

        vkCmdDraw(activeBuffer, dynVertexBuffer.dataCount, 1, 0, 0);


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
    ScriptThreadSafelyClosed = CloseWrenScriptThread(ScriptHandle);
    if (ScriptThreadSafelyClosed) printf("Safely Closed Wren Thread: %i\n", ScriptThreadSafelyClosed);

    glfwTerminate();


    return 0;
}
