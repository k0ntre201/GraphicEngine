#version 450 core

#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inNormal;

layout (std140, binding = 0) uniform CameraMatrices
{
    mat4 view;
    mat4 projection;
} cameraMatrices;

layout (std140) uniform SolidColorModelDescriptor
{
    mat4 modelMatrix;
    mat4 normalMatrix;
} solidColorModelDescriptor;

layout (location = 0) out vec3 position;
layout (location = 1) out vec3 normal;

void main()
{
    normal = normalize(mat3(solidColorModelDescriptor.normalMatrix) * inNormal);
    position = vec3(solidColorModelDescriptor.modelMatrix * vec4(inPosition, 1.0));
    gl_Position = cameraMatrices.projection * cameraMatrices.view * vec4(position, 1.0);
}