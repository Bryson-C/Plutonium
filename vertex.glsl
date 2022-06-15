#version 450

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 col;
/*
layout(binding = 0) uniform UBO {
    vec3 xyz;
} ubo;
layout(push_constant) uniform CONSTANT {
    vec3 xyz;
} pushConstant;
*/

layout(location = 0) out vec4 colorToFragment;


void main() {
    gl_Position = vec4(pos /*+ pushConstant.xyz*/, 1.0);


    colorToFragment = vec4(col/* * ubo.xyz*/, 1.0);
}