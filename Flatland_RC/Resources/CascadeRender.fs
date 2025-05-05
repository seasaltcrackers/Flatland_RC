#version 450 core

in vec2 fragTexCoord;

out vec4 color;

uniform sampler2D worldTexture;
uniform vec2 worldTextureDimensions;

uniform sampler2D cascadeTexture;
uniform vec2 cascadeTextureDimensions;

uniform ivec2 cascade0AngleResolution;
uniform ivec2 cascade0Dimensions;

void main(void)
{
	vec4 worldColour = texture(worldTexture, fragTexCoord);

    vec2 topLeft = cascade0Dimensions * fragTexCoord;
    vec4 combinedRadiance = vec4(0.0f, 0.0f, 0.0f, 0.0f);

    for (int y = 0; y < cascade0AngleResolution.y; ++y)
    {
        for (int x = 0; x < cascade0AngleResolution.x; ++x)
        {
            vec2 samplePosition = topLeft + vec2(x, y);
	        vec4 radiance = texture(cascadeTexture, samplePosition / cascadeTextureDimensions);
            combinedRadiance += radiance;
        }
    }

    color = combinedRadiance / (cascade0AngleResolution.x * cascade0AngleResolution.y);
}