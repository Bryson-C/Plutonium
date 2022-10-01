#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoords;
layout(location = 3) in uint textureId;


layout(set = 0, binding = 0) uniform UBO {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec2 outTexCoords;
layout(location = 2) out uint outTextureId;


void main() {

    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition.x, inPosition.y, inPosition.z, 1.0);

    outColor = vec4(inColor, 1.0);
    outTexCoords = inTexCoords;
    outTextureId = textureId;
}