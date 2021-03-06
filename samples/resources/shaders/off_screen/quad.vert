#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec2 inUV;

layout(set = 0, binding = 0) uniform BuildIn {
    mat4 matrixObjectToNDC;
} _buildIn;

layout (location = 0) out vec2 outUV;

out gl_PerVertex
{
    vec4 gl_Position;
};

void main() 
{
    outUV = inUV;
    gl_Position = _buildIn.matrixObjectToNDC * vec4(inPos.xyz, 1.0);
}
