#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoords;


layout(set = 0, binding = 0) uniform UBO {
    float x;
    float y;
} UniformBufferObject;

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec2 outTexCoords;


void main() {

    vec3 position = vec3(inPosition.x + UniformBufferObject.x, inPosition.y + UniformBufferObject.y, inPosition.z);

    gl_Position = vec4(position, 1.0);
    outColor = vec4(inColor, 1.0);
    outTexCoords = inTexCoords;
}