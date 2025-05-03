#version 450 core

layout (location = 0) in vec2 position;
layout (location = 1) in vec2 texCoord;

out vec2 fromProbeCoord;
out vec2 toProbeCoord;

out vec2 fromPixelCoord;
out vec2 toPixelCoord;

uniform vec2 cascadeTextureDimensions;
 
// { offset, scale }
uniform vec2 fromHorizontalTransform;
uniform vec2 toHorizontalTransform; 

uniform ivec2 mergeFromProbeResolution;
uniform ivec2 mergeToProbeResolution;

uniform ivec2 mergeFromAngleResolution;
uniform ivec2 mergeToAngleResolution;

void main()
{
    // 0.0f -> 1.0f transformed coordinates
    float fromTransformedTexCoordX = texCoord.x * fromHorizontalTransform.y + fromHorizontalTransform.x; // Then transform to match the requested horizontal cascade slice    
    float toTransformedTexCoordX = texCoord.x * toHorizontalTransform.y + toHorizontalTransform.x; // Then transform to match the requested horizontal cascade slice    
    
	gl_Position = vec4(position, 0.0f, 1.0f);
    gl_Position.x = (toTransformedTexCoordX * 2.0f) - 1.0f; // Remap to -1.0f -> 1.0f

    fromProbeCoord = texCoord * mergeFromProbeResolution;
    toProbeCoord = texCoord * mergeToProbeResolution;

    fromPixelCoord = vec2(fromTransformedTexCoordX, texCoord.y) * cascadeTextureDimensions;
    toPixelCoord = vec2(toTransformedTexCoordX, texCoord.y) * cascadeTextureDimensions;
}