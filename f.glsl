#version 450

layout(location = 0) in vec4 inColor;
layout(location = 1) in vec2 inTexCoords;

layout(location = 0) out vec4 outColor;

layout(set = 1, binding = 0) uniform sampler2D tex;

void main() {
    //outColor = vec4(inTexCoords, 0.0, 1.0);
    outColor = texture(tex, inTexCoords);
    //outColor = inColor;
}