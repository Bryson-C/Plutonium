#version 450

layout(location = 0) in vec4 col;
layout(location = 0) out vec4 colorToScreen;

void main() {
    colorToScreen = col;
}