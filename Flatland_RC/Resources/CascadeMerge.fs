#version 450 core

#define M_PI 3.1415926535897932384626433832795

in vec2 fromProbeCoord;
in vec2 toProbeCoord;

in vec2 fromPixelCoord;
in vec2 toPixelCoord;

out vec4 color;

uniform bool bilinearFix;

uniform int mergeToCascade;

uniform float mergeFromLeftPositionX;
uniform float mergeToLeftPositionX;

uniform ivec2 mergeFromProbeResolution;
uniform ivec2 mergeToProbeResolution;

uniform ivec2 mergeFromAngleResolution;
uniform ivec2 mergeToAngleResolution;

uniform sampler2D cascadeTexture;
uniform vec2 cascadeTextureDimensions;

vec4 mergeThing(vec4 near, vec4 far)
{   
    vec3 radiance = near.rgb + (far.rgb * near.a);
    return vec4(radiance, near.a * far.a);
}

vec4 bilinearWeights(vec2 ratio)
{
    return vec4(
        (1.0f - ratio.x) * (1.0f - ratio.y),
        ratio.x * (1.0f - ratio.y),
        (1.0f - ratio.x) * ratio.y,
        ratio.x * ratio.y
    );
}

vec4 mergeIntervals(vec4 toRadiance, ivec2 toProbeCoordinates, ivec2 fromProbeCoordinates, int mergeToAngleIndex)
{
    vec4 weights = vec4(1.0f, 1.0f, 1.0f, 1.0f) / 4.0f;

    if (bilinearFix)
    {
        vec2 toProbeInFromCoordSpace = (vec2(toProbeCoordinates * mergeToAngleResolution) * vec2(0.5f, 1.0f)) / mergeFromAngleResolution;
        vec2 bilinearRatio = fract(toProbeInFromCoordSpace) + vec2(0.25f, 0.25f);
        //return vec4(bilinearRatio, 0.0f, 1.0f);
        weights = bilinearWeights(bilinearRatio);
    }
    
    vec4 radiance = vec4(0.0f, 0.0f, 0.0f, 0.0f);

    // Loop over every probe to merge in the from cascade
    for (int yOffset = 0; yOffset < 2; ++yOffset)
    {
        for (int xOffset = 0; xOffset < 2; ++xOffset)
        {
            ivec2 probeOffset = ivec2(xOffset, yOffset);
            ivec2 probeCoordinate = (fromProbeCoordinates + probeOffset);
            
            if (any(greaterThanEqual(probeCoordinate, mergeFromProbeResolution)))
                continue;

            vec2 probeTopLeftPosition = probeCoordinate * mergeFromAngleResolution;
            probeTopLeftPosition.x += mergeFromLeftPositionX;

            // Loop over every direction to sample in the from cascade
            for (int directionOffset = -1; directionOffset < 1; ++directionOffset)
            {
                int mergeFromAngleIndex = mergeToAngleIndex * 2 + directionOffset;

                ivec2 mergeFromAngleCoord = ivec2(
                    mergeFromAngleIndex % mergeFromAngleResolution.x,
                    floor(mergeFromAngleIndex / float(mergeFromAngleResolution.x)));

                vec2 samplePosition = probeTopLeftPosition + mergeFromAngleCoord;
                vec4 fromRadiance = texture(cascadeTexture, samplePosition / cascadeTextureDimensions);
                radiance += mergeThing(toRadiance, fromRadiance) * weights[yOffset * 2 + xOffset];
            }
        }
    }

    // Normalize
    return radiance / 2.0f;
}

void main(void)
{
    // Get pixel index
    ivec2 angleCoord = ivec2(toPixelCoord);
    angleCoord %= mergeToAngleResolution;

    int toAngleIndex = angleCoord.y * mergeToAngleResolution.x + angleCoord.x;
    
    vec4 near = texture(cascadeTexture, toPixelCoord / cascadeTextureDimensions);
    vec4 far = mergeIntervals(near, ivec2(floor(toProbeCoord)), ivec2(floor(fromProbeCoord)), toAngleIndex);
    
    color = far;
}
