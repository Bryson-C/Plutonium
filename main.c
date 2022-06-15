#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

//#include "Abstraction/Abstractions.h"
#include "RenderState/RenderState.h"


#define STB_IMAGE_IMPLEMENTATION 1
#include "lib/stb_image.h"
#include "vulkan/vulkan.h"
#include "shaderc.h"
/*#define TINYOBJLOADER_IMPLEMENTATION 1
#include "../TinyOBJ.hpp"*/


#include "status.h"
#include "glfw3.h"

typedef uint32_t U32;


static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData) {

    if (messageSeverity > VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
        printf("[Vk]: %s\n\n",pCallbackData->pMessage);

    return VK_FALSE;
}

const char* shaderCompilationErrorCodeToString(int errorCode) {
    printf("(Code: %i): ", errorCode);
    switch (errorCode) {
        case shaderc_compilation_status_success: return "Success";
        case shaderc_compilation_status_invalid_stage: return "Failure To Deduce Shader Stage"; // error stage deduction
        case shaderc_compilation_status_compilation_error: return "Compilation Error";
        case shaderc_compilation_status_internal_error : return "Unexpected Failure"; // unexpected failure
        case shaderc_compilation_status_null_result_object: return "Null Result Objects";
        case shaderc_compilation_status_invalid_assembly: return "Invalid Assembly";
        case shaderc_compilation_status_validation_error: return "Validation Error";
        case shaderc_compilation_status_transformation_error : return "Transformation Error";
        case shaderc_compilation_status_configuration_error: return "Configuration Error";
        default: return "Unknown Error Code";
    }
}


void createNewQuadViaVertices(U32 vertexCount, vertex* vertices, float x, float y, float w, float h, Float3 color) {
    vertices[vertexCount] = (vertex){{x,y,0.0f}, color, {0,0}};
    vertices[vertexCount+1] = (vertex){{x + w,y,0.0f}, color, {0,0}};
    vertices[vertexCount+2] = (vertex){{x + w,y + h,0.0f}, color, {0,0}};
    vertices[vertexCount+3] = (vertex){{x,y + h,0.0f}, color, {0,0}};
}

void DrawVertexBuffer(Buffer vertexBuffer, Buffer IndexBuffer, U32 vertexCount) {

}


int main() {

    const Float3 softBlue = {0.1f,0.3f,1.0f};
    const Float3 softRed = {1.0f,0.3f,0.1f};
    const Float3 softGreen = {0.1f,1.0f,0.3f};


    U32 vertexAllocCount = 4 * 100;
    U32 shapeCount = 0;
    vertex* vertices = (vertex*)malloc(sizeof(vertex) * vertexAllocCount);

    for (; shapeCount < vertexAllocCount/4; shapeCount++) {
        srand(clock() * shapeCount);
        createNewQuadViaVertices(shapeCount*4, vertices,
                                 ((float)shapeCount * 0.01f) - 1.0f,
                                 ((float)shapeCount * 0.01f) - 1.0f,
                                 0.05f,
                                 0.05f,
                                 (shapeCount%2==0) ? softBlue : softRed);
    }

    U32 indexCount = 6;
    U32 indices[] = {
            0, 1, 2, 2, 3, 0
    };


    ApplyShader(VK_SHADER_STAGE_VERTEX_BIT, "D:\\Plutonium\\vertex.spv", "main");
    ApplyShader(VK_SHADER_STAGE_FRAGMENT_BIT, "D:\\Plutonium\\fragment.spv", "main");

    U32 bindingCount = 1;
    VkVertexInputBindingDescription bindings[] = {
            createVertexBinding(0, VK_VERTEX_INPUT_RATE_VERTEX, sizeof(vertex))
    };

    U32 attributeCount = 2;
    VkVertexInputAttributeDescription attributes[] = {
            createVertexAttribute(0, 0, (VkFormat)shaderVec3, offsetof(vertex,pos)),
            createVertexAttribute(0, 1, (VkFormat)shaderVec3, offsetof(vertex,col)),
    };
    ApplyVertexInput(attributeCount, attributes, bindingCount, bindings);

    ApplyDescriptorLayouts(0, VK_NULL_HANDLE);
    ApplyPushConstants(0, VK_NULL_HANDLE);


    CreateRenderState();

    Buffer VertexBuffer = RequestBuffer(sizeof(vertex) * vertexAllocCount, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, vertices, true);
    Buffer IndexBuffer = RequestBuffer(sizeof(U32) * indexCount, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, indices, true);

    const U32 descriptorCount = 2;
    Buffer UniformBuffers[2] = {
            RequestBuffer(sizeof(UBO), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_NULL_HANDLE, false),
            RequestBuffer(sizeof(UBO), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_NULL_HANDLE, false),
    };
    VkDescriptorSetLayoutBinding binding = createNewBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, descriptorCount, VK_SHADER_STAGE_FRAGMENT_BIT);
    VkDescriptorSetLayout layout = createDescriptorLayout(VRS.device, 1, &binding);
    VkDescriptorPool descriptorPool;
    VkDescriptorSet* descriptorSets = createDescriptorSet(VRS.device, 2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, layout, &descriptorPool);
    for (U32 i = 0; i < descriptorCount; i++)
        writeDescriptor(VRS.device, descriptorSets[i], VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, &UniformBuffers[i].bufferInfo, VK_NULL_HANDLE);


    ApplyDescriptorLayouts(1, &layout);
    ApplyPushConstants(0, VK_NULL_HANDLE);

    CreateRenderStateRenderer(); // recreate

    while (!glfwWindowShouldClose(VRS.window)) {
        glfwPollEvents();
        BeginDraw();


        float color = sin((double)clock()/1000)/1.0f + 0.5f;
        UBO ubo;
        ubo.xyz = (Float3){color, color, color};

        void* data;
        vkMapMemory(VRS.device, UniformBuffers[VRS.activeFrame].memory, 0, sizeof(UBO), 0, &data);
        memcpy(data, &ubo, sizeof(UBO));
        vkUnmapMemory(VRS.device, UniformBuffers[VRS.activeFrame].memory);


        CommandBuffer currentBuffer = VRS.renderBuffers[VRS.imageIndex];
        VkDeviceSize offsets[] = {0};
        vkCmdBindDescriptorSets(currentBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                VRS.pipelineLayout,
                                0,
                                1,
                                &descriptorSets[VRS.activeFrame],
                                0, VK_NULL_HANDLE);
        vkCmdBindVertexBuffers(currentBuffer, 0, 1, &VertexBuffer.buffer, offsets);
        vkCmdBindIndexBuffer(currentBuffer, IndexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
        for (int32_t i = 0; i < shapeCount; i++) {
            vkCmdDrawIndexed(currentBuffer, indexCount, 1, 0, i*4, 0);
        }

        SubmitDraw();
        glfwSwapBuffers(VRS.window);
    }


    DestroyRenderState();

    return 0;
}

