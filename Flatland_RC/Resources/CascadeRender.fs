#version 450 core

in vec2 fragTexCoord;

out vec4 color;

uniform bool bilinearFix;

uniform vec2 worldTextureDimensions;

uniform sampler2D cascadeTexture;
uniform vec2 cascadeTextureDimensions;

uniform ivec2 cascade0AngleResolution;
uniform ivec2 cascade0ProbeResolution;

vec4 bilinearWeights(vec2 ratio)
{
    return vec4(
        (1.0f - ratio.x) * (1.0f - ratio.y),
        ratio.x * (1.0f - ratio.y),
        (1.0f - ratio.x) * ratio.y,
        ratio.x * ratio.y
    );
}

vec2 CalculateRatio(ivec2 probeCoordinates)
{
    probeCoordinates -= 1;
    probeCoordinates %= 2;
    int index = probeCoordinates.y * 2 + probeCoordinates.x;

    const vec2 test[4] = { vec2(0.25f, 0.25f), vec2(0.75f, 0.25f), vec2(0.25f, 0.75f), vec2(0.75f, 0.75f) };
    return test[index];
}

ivec2 TransformThing(ivec2 probeCoordinate)
{
    return ivec2(floor((probeCoordinate - 1) * 0.5f));
}

void main(void)
{
    ivec2 worldPosition = ivec2(floor(worldTextureDimensions * fragTexCoord));

    // TODO: This shouldnt assume 2x scaling from world space to cascade0
    ivec2 bottomLeftProbeCoordinate = TransformThing(worldPosition);

    vec4 combinedRadiance = vec4(0.0f, 0.0f, 0.0f, 0.0f);

    vec4 weights = vec4(1.0f, 1.0f, 1.0f, 1.0f);
    int probeSampleAmount = bilinearFix ? 2 : 1;

    if (bilinearFix)
    {
        // TODO: This shouldnt assume 2x scaling from world space to cascade0
        vec2 ratioThing = CalculateRatio(worldPosition);
        weights = bilinearWeights(ratioThing);
    }
    
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
    color.rgb = pow(color.rgb, 1 / vec3(2.2f, 2.2f, 2.2f)); // TODO: Option to turn off SRGB Calculation
}