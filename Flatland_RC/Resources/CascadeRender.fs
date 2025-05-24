#version 450 core

in vec2 fragTexCoord;

out vec4 color;

uniform int sampleType;
uniform bool enableSRGB;

uniform vec2 worldTextureDimensions;

uniform sampler2D cascadeTexture;
uniform vec2 cascadeTextureDimensions;

uniform ivec2 cascade0AngleResolution;
uniform ivec2 cascade0ProbeResolution;

vec4 BilinearWeights(vec2 ratio)
{
    return vec4(
        (1.0f - ratio.x) * (1.0f - ratio.y),
        ratio.x * (1.0f - ratio.y),
        (1.0f - ratio.x) * ratio.y,
        ratio.x * ratio.y
    );
}

vec2 WorldPositionToProbeCoordinate(vec2 worldPosition, ivec2 probeResolution)
{
    vec2 probeSpacing = worldTextureDimensions / probeResolution;
    return (worldPosition / probeSpacing) - 0.5f;
}

void main(void)
{
    vec2 worldPosition = worldTextureDimensions * fragTexCoord;
    vec2 probeCoordinate = WorldPositionToProbeCoordinate(worldPosition, cascade0ProbeResolution);

    ivec2 bottomLeftProbeCoordinate = ivec2(0, 0);
    vec4 weights = vec4(0, 0, 0, 0);
    int probeSampleAmount = 0;

    if (sampleType == 0)
    {
        // Nearest Neighbour
        bottomLeftProbeCoordinate = ivec2(round(probeCoordinate));
        weights = vec4(1.0f, 1.0f, 1.0f, 1.0f);
        probeSampleAmount = 1;
    }
    else if (sampleType == 1)
    {
        // Average
        bottomLeftProbeCoordinate = ivec2(floor(probeCoordinate));
        weights = vec4(0.25f, 0.25f, 0.25f, 0.25f);
        probeSampleAmount = 2;
    }
    else if (sampleType == 2)
    {
        // Bilinear Interpolation
        bottomLeftProbeCoordinate = ivec2(floor(probeCoordinate));
        weights = BilinearWeights(fract(probeCoordinate));
        probeSampleAmount = 2;
    }
    
    vec4 combinedRadiance = vec4(0.0f, 0.0f, 0.0f, 0.0f);

    for (int probeOffsetY = 0; probeOffsetY < probeSampleAmount; ++probeOffsetY)
    {
        for (int probeOffsetX = 0; probeOffsetX < probeSampleAmount; ++probeOffsetX)
        {
            ivec2 probeCoordinate = bottomLeftProbeCoordinate + ivec2(probeOffsetX, probeOffsetY);
            
            if (any(lessThan(probeCoordinate, ivec2(0, 0))) || any(greaterThanEqual(probeCoordinate, cascade0ProbeResolution)))
                continue;

            ivec2 probePositionOffsetBottomLeft = probeCoordinate * cascade0AngleResolution;

            for (int directionOffsetY = 0; directionOffsetY < cascade0AngleResolution.y; ++directionOffsetY)
            {
                for (int directionOffsetX = 0; directionOffsetX < cascade0AngleResolution.x; ++directionOffsetX)
                {
                    vec2 samplePosition = probePositionOffsetBottomLeft + vec2(directionOffsetX, directionOffsetY);

                    vec4 radiance = texture(cascadeTexture, samplePosition / cascadeTextureDimensions);
                    combinedRadiance += radiance * weights[probeOffsetY * 2 + probeOffsetX];
                }
            }
        }
    }

    color = combinedRadiance / (cascade0AngleResolution.x * cascade0AngleResolution.y);

    if (enableSRGB)
        color.rgb = pow(color.rgb, 1 / vec3(2.2f, 2.2f, 2.2f)); // TODO: Option to turn off SRGB Calculation
}