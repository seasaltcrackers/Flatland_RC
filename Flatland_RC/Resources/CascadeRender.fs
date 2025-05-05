#version 450 core

in vec2 fragTexCoord;

out vec4 color;

uniform sampler2D worldTexture;
uniform vec2 worldTextureDimensions;

uniform sampler2D cascadeTexture;
uniform vec2 cascadeTextureDimensions;

uniform ivec2 cascade0AngleResolution;
uniform ivec2 cascade0Dimensions;

vec4 bilinearWeights(vec2 ratio) {
    return vec4(
        (1.0f - ratio.x) * (1.0f - ratio.y),
        ratio.x * (1.0f - ratio.y),
        (1.0f - ratio.x) * ratio.y,
        ratio.x * ratio.y
    );
}

void main(void)
{
	vec4 worldColour = texture(worldTexture, fragTexCoord);

    vec2 cascadePosition = cascade0Dimensions * fragTexCoord;
    ivec2 topLeftProbeCoordinate = ivec2(floor(cascadePosition / cascade0AngleResolution));
    vec2 topLeftProbePosition = topLeftProbeCoordinate * cascade0AngleResolution;

    vec4 combinedRadiance = vec4(0.0f, 0.0f, 0.0f, 0.0f);

    vec2 toProbeInFromCoordSpace = (cascadePosition / cascade0AngleResolution);
    vec2 bilinearRatio = fract(toProbeInFromCoordSpace);

    color = vec4(bilinearRatio, 0.0f, 1.0f);
    return;

    vec4 weights = bilinearWeights(bilinearRatio);
    
    for (int probeOffsetY = 0; probeOffsetY < 2; ++probeOffsetY)
    {
        for (int probeOffsetX = 0; probeOffsetX < 2; ++probeOffsetX)
        {
            vec2 probeCoordinateOffset = vec2(probeOffsetX, probeOffsetY);
            vec2 probePositionOffsetTopLeft = topLeftProbePosition + probeCoordinateOffset * cascade0AngleResolution;
            
            for (int directionOffsetY = 0; directionOffsetY < cascade0AngleResolution.y; ++directionOffsetY)
            {
                for (int directionOffsetX = 0; directionOffsetX < cascade0AngleResolution.x; ++directionOffsetX)
                {
                    vec2 samplePosition = probePositionOffsetTopLeft + vec2(directionOffsetX, directionOffsetY);
                    vec4 radiance = texture(cascadeTexture, samplePosition / cascadeTextureDimensions);
                    combinedRadiance += radiance * weights[probeOffsetY * 2 + probeOffsetX];
                }
            }
        }
    }

    color = combinedRadiance / (cascade0AngleResolution.x * cascade0AngleResolution.y);
}