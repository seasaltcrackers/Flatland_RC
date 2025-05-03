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
    float transformedTexCoordX = texCoord.x * horizontalTransform.y + horizontalTransform.x; // Then transform to match the requested horizontal cascade slice    
    
	gl_Position = vec4(position, 0.0f, 1.0f);
    gl_Position.x = (transformedTexCoordX * 2.0f) - 1.0f; // Remap to -1.0f -> 1.0f;

    fromProbeCoord
    toProbeCoord

    fromPixelCoord
    toPixelCoord


	fragTexCoord = texCoord;
    fragTexCoord.x = transformedTexCoordX;

    fragPixelCoord = fragTexCoord * cascadeTextureDimensions;
}