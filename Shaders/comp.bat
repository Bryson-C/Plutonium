@echo off
glslc -fshader-stage=vertex v.glsl -o v.spv
glslc -fshader-stage=fragment f.glsl -o f.spv