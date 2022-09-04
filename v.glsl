#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;


layout(set = 0, binding = 0) uniform UBO {
    float x;
    float y;
} UniformBufferObject;

layout(location = 0) out vec4 outColor;


void main() {

    vec3 position = vec3(inPosition.x + UniformBufferObject.x, inPosition.y + UniformBufferObject.y, inPosition.z);

    gl_Position = vec4(position, 1.0);
    outColor = vec4(inColor, 1.0);
}