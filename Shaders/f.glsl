#version 450

layout(location = 0) in vec4 inColor;
layout(location = 1) in vec2 inTexCoords;
layout(location = 2) in flat uint inTexId;

layout(location = 0) out vec4 outColor;

layout(set = 1, binding = 0) uniform sampler textureSampler;
layout(set = 1, binding = 1) uniform texture2D textures[8];


void main() {
    if (inTexId == 0)
        outColor = inColor;
    else
        outColor = inColor * texture(sampler2D(textures[inTexId], textureSampler), inTexCoords);
}