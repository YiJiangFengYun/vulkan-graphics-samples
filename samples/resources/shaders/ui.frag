#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (set = 1, binding = 0) uniform sampler2D texSampler;

layout (location = 0) in vec2 inUV;
layout (location = 1) in vec4 inColor;

layout (location = 0) out vec4 outColor;

void main() 
{
    outColor = inColor * texture(texSampler, inUV);
    // outColor = inColor;
    // outColor = vec4(1.0, 1.0, 1.0, 0.3);
}