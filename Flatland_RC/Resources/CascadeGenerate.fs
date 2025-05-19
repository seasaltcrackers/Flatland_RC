#version 450 core

#define M_PI 3.1415926535897932384626433832795

in vec2 fragTexCoord;
in vec2 fragPixelCoord;

out vec4 color;

uniform float cascade0IntervalLength;
uniform ivec2 cascade0ProbeResolution;
uniform ivec2 cascade0AngleResolution;
uniform vec2 cascadeTextureDimensions;

uniform sampler2D worldTexture;
uniform vec2 worldTextureDimensions;


// STOLEN
vec4 Raycast(vec2 rayOrigin, vec2 rayDirection, float intervalMin, float intervalMax)
{
    vec2 from = rayOrigin + rayDirection * intervalMin;
    vec2 to = rayOrigin + rayDirection * intervalMax;

    vec2 delta = abs(to - from);
    ivec2 current = ivec2(floor(from));

    int n = 1;
    ivec2 inc = ivec2(0, 0);
    float error = 0;

    if (delta.x == 0)
    {
        inc.x = 0;
        error = 99999999.0f;
    }
    else if (to.x > from.x)
    {
        inc.x = 1;
        n += int(floor(to.x)) - current.x;
        error = (floor(from.x) + 1 - from.x) * delta.y;
    }
    else
    {
        inc.x = -1;
        n += current.x - int(floor(to.x));
        error = (from.x - floor(from.x)) * delta.y;
    }

    if (delta.y == 0)
    {
        inc.y = 0;
        error = -99999999.0f;
    }
    else if (to.y > from.y)
    {
        inc.y = 1;
        n += int(floor(to.y)) - current.y;
        error -= (floor(from.y) + 1 - from.y) * delta.x;
    }
    else
    {
        inc.y = -1;
        n += current.y - int(floor(to.y));
        error -= (from.y - floor(from.y)) * delta.x;
    }

    vec3 color = vec3(0.0f, 0.0f, 0.0f);

    for (; n > 0; --n)
    {
        if (any(lessThan(current, vec2(0, 0))) || any(greaterThanEqual(current, worldTextureDimensions)))
            return vec4(color, 1.0f); // 1 means it hit nothing

        vec4 sampledColour = texture(worldTexture, current / (worldTextureDimensions - 1));
        color = sampledColour.rgb;

        if (sampledColour.a == 1.0f)
            return vec4(color, 0.0f); // 0 means it hit something

        if (error > 0)
        {
            current.y += inc.y;
            error -= delta.x;
        }
        else
        {
            current.x += inc.x;
            error += delta.y;
        }
    }
    
    return vec4(color.rgb, 1.0f); // 1 means it hit nothing
}

//vec4 Raycast(vec2 rayOrigin, vec2 rayDirection, float intervalMin, float intervalMax)
//{
//    int steps = max(2, int(round((intervalMax - intervalMin) * 3.0f)));
//    vec4 color = vec4(0.0f, 0.0f, 0.0f, 0.0f);
//
//    for (int i = 0; i < steps; ++i)
//    {
//        float rayDistanceT = float(i) / (steps - 1);
//        float rayDistance = mix(intervalMin, intervalMax, rayDistanceT);
//        vec2 samplePosition = rayOrigin + rayDirection * rayDistance;
//
//        if (any(lessThan(samplePosition, vec2(0, 0))) || any(greaterThan(samplePosition, worldTextureDimensions)))
//            return vec4(color.rgb, 0.0f);
//
//        color = texture(worldTexture, samplePosition / worldTextureDimensions);
//
//        if (color.a == 1.0f)
//            return color;
//    }
//
//    return vec4(color.rgb, 0.0f);
//}

float IntervalScale(int cascade) 
{
    if (cascade <= 0) 
        return 0.0;

    /* Scale interval by 2x each cascade */
    return float(1 << (cascade));
}

vec2 CalculateIntervalMinMax(int cascade)
{
    return cascade0IntervalLength * vec2(IntervalScale(cascade), IntervalScale(cascade + 1));
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

vec2 ProbeCoordinateToWorldPosition(ivec2 probeResolution, ivec2 coords)
{
    // Divide by resolution + 1 so the grid sits away from the edges of the screen
    vec2 probeSpacing = worldTextureDimensions / (probeResolution + 1);

    // Add 1 here because otherwise it would be the coordinates would start flush from the top left corner
    return probeSpacing * (coords + 1);
}

ivec2 CascadePositionToProbeCoordinate(int cascade, ivec2 angularResolution, vec2 position)
{
    float cascadeStartXNormalized = 1.0f - pow(0.5f, cascade);
    position.x -= (cascadeStartXNormalized * cascadeTextureDimensions.x);
    return ivec2(floor(position / angularResolution));
}

void main(void)
{
    // Get the cascade index of the current pixel based on how far across horizontally it is
    // Assumes width is 2 * cascade 0 width
    int cascade = int(floor(log(1.0f - fragTexCoord.x) / log(0.5f)));

    ivec2 probeResolution = CalculateProbeResolution(cascade);
    ivec2 angularResolution = CalculateAngleResolution(cascade);
    vec2 intervalMinMax = CalculateIntervalMinMax(cascade);

    ivec2 angleCoord = ivec2(fragPixelCoord);
    angleCoord %= angularResolution;

    int angleIndex = angleCoord.y * angularResolution.x + angleCoord.x;
    
    // t does not reach 1.0f as angleIndex is 0 based while resolution is not
    // This is on purpose so we don't raycast 0 & 360 angle
    float angleT = angleIndex / float(angularResolution.x * angularResolution.y);
    
    // Adding 45 degrees so that e.g. the 4 raycasts are offset rather than on the axis aligned
    float rayAngle = angleT * (M_PI * 2.0f) + (M_PI * 0.25f);
    vec2 rayDirection = vec2(cos(rayAngle), sin(rayAngle));
    
    ivec2 probeCoordinate = CascadePositionToProbeCoordinate(cascade, angularResolution, fragPixelCoord);
    vec2 probeCenterWorldPosition = ProbeCoordinateToWorldPosition(probeResolution, probeCoordinate);

    //color = vec4(probeCenterWorldPosition / worldTextureDimensions, 0.0f, 1.0f);
    //return;

    // Raycast in the direction 
    vec4 radiance = Raycast(probeCenterWorldPosition, rayDirection, intervalMinMax.x, intervalMinMax.y);
    color = radiance;//vec4(radiance, 1.0f);

    //color.rgb = color.a == 1.0f ? vec3(1.0f, 1.0f, 1.0f) : vec3(0.0f, 0.0f, 0.0f);

    //color = vec4(vec2(probePosition) / worldTextureDimensions, 0.0f, 1.0f);
    //color = vec4((probeResolution / 32.0f), 0.0f, 1.0f);

    //color = vec4((cascade / 10.0f).rrr, 1.0f);

    // Show cascade angular pixel coordinates:
    //color = vec4((vec2(pixelCoord) / angularResolution), 0.0f, 1.0f);
}
