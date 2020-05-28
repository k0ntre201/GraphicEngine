#version 430 core

#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inColor;
layout (location = 2) in vec2 inTexCoord;

layout(std140, binding = 0) uniform MVP
{
    mat4 vp; 
} mvp;

layout (location = 0) out vec3 fragColor;
layout (location = 1) out vec2 texCoord;

void main()
{
    fragColor = inColor;
    texCoord = inTexCoord;
    gl_Position = mvp.vp * vec4(inPosition, 1.0);
}