@echo off
echo Compiling shaders...

:: Fragment shaders
glslc res\shaders\simple_shader.frag -o res\shaders\simple_shader.frag.spv
glslc res\shaders\billboard_shader.frag -o res\shaders\billboard_shader.frag.spv

:: Vertex shaders
glslc res\shaders\simple_shader.vert -o res\shaders\simple_shader.vert.spv
glslc res\shaders\billboard_shader.vert -o res\shaders\billboard_shader.vert.spv

echo Complete.