#version 450 core

#define M_PI 3.1415926535897932384626433832795

in vec2 fragTexCoord;
in vec2 fragPixelCoord;

out vec4 color;

uniform ivec2 cascade0ProbeResolution;
uniform ivec2 cascade0AngleResolution;

uniform sampler2D worldTexture;
uniform vec2 worldTextureDimensions;

vec3 Raycast(vec2 position, vec2 direction, float intervalMin, float intervalMax)
{
    for (float distance = intervalMin; distance <= intervalMax; distance += 1.0f)
    {
        vec2 samplePosition = position + direction * distance;
        vec4 color = texture(worldTexture, samplePosition / worldTextureDimensions);

        if (color.a == 1.0f)
            return color.rgb;
    }

    return vec3(0.0f, 0.0f, 0.0f);
}

ivec2 CalculateProbeResolution(int cascade)
{
    return ivec2(
        ceil(cascade0ProbeResolution.x / pow(2.0f, cascade)),
        ceil(cascade0ProbeResolution.y / pow(2.0f, cascade)));
}

ivec2 CalculateAngleResolution(int cascade)
{
    return ivec2(
        cascade0AngleResolution.x,
        cascade0AngleResolution.y * pow(2, cascade));
}

void main(void)
{
    //cascade0Resolution ;

    // Get the cascade index of the current pixel based on how far across horizontally it is
    // Assumes width is 2 * cascade 0 width
    int cascade = int(floor(log(1.0f - fragTexCoord.x) / log(0.5f)));

    ivec2 probeResolution = CalculateProbeResolution(cascade);
    ivec2 angularResolution = CalculateAngleResolution(cascade);

    float intervalMin = 2.0f;
    float intervalMax = 8.0f;


    ivec2 pixelCoord = ivec2(fragPixelCoord);
    pixelCoord %= angularResolution;

    int angleIndex = pixelCoord.y * angularResolution.x + pixelCoord.x;
    
    // t does not reach 1.0f as angleIndex is 0 based while resolution is not
    // This is on purpose so we don't raycast 0 & 360 angle
    float t = angleIndex / float(angularResolution.x * angularResolution.y);
    
    // Adding 45 degrees so that e.g. the 4 raycasts are offset rather than on the axis aligned
    float angle = t * (M_PI * 2.0f) + (M_PI * 0.25f);
    vec2 direction = vec2(cos(angle), sin(angle));
    
    ivec2 probeCoordinate = ivec2(floor(fragPixelCoord / angularResolution));
    vec2 probePosiiton = probeCoordinate ;

    // Raycast in the direction 
    vec3 radiance = Raycast(fragPixelCoord, direction, intervalMin, intervalMax);
    color = vec4(radiance, 1.0f);
    //color = vec4((cascade / 10.0f).rrr, 1.0f);

    // Show cascade angular pixel coordinates:
    //color = vec4((vec2(pixelCoord) / angularResolution), 0.0f, 1.0f);
}
