#version 450 core

#define M_PI 3.1415926535897932384626433832795

in vec2 toProbeCoordInput;
in vec2 toPixelCoordInput;

out vec4 color;

uniform bool doMerge;
uniform bool bilinearFix;

uniform vec2 mergeToIntervalMinMax;

uniform float mergeFromLeftPositionX;

uniform ivec2 mergeFromProbeResolution;
uniform ivec2 mergeToProbeResolution;

uniform ivec2 mergeFromAngleResolution;
uniform ivec2 mergeToAngleResolution;

uniform sampler2D cascadeTexture;
uniform vec2 cascadeTextureDimensions;

uniform sampler2D worldTexture;
uniform vec2 worldTextureDimensions;

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

vec2 CalculateRatio(ivec2 probeCoordinates)
{
    probeCoordinates -= 1;
    probeCoordinates %= 2;
    int index = probeCoordinates.y * 2 + probeCoordinates.x;

    const vec2 test[4] = { vec2(0.25f, 0.25f), vec2(0.75f, 0.25f), vec2(0.25f, 0.75f), vec2(0.75f, 0.75f) };
    return test[index];
}


vec4 mergeIntervals(vec4 toRadiance, ivec2 toProbeCoordinates, ivec2 fromProbeCoordinates, int mergeToAngleIndex)
{
    vec4 weights = vec4(0.25f, 0.25f, 0.25f, 0.25f);

    if (bilinearFix)
    {
        vec2 ratioThing = CalculateRatio(toProbeCoordinates);
        weights = bilinearWeights(ratioThing);
    }

    vec4 radiance = vec4(0.0f, 0.0f, 0.0f, 0.0f);

    // Loop over every probe to merge in the from cascade
    for (int yOffset = 0; yOffset < 2; ++yOffset)
    {
        for (int xOffset = 0; xOffset < 2; ++xOffset)
        {
            ivec2 probeOffset = ivec2(xOffset, yOffset);
            ivec2 probeCoordinate = (fromProbeCoordinates + probeOffset);
            
            if (any(lessThan(probeCoordinate, ivec2(0, 0))) || any(greaterThanEqual(probeCoordinate, mergeFromProbeResolution)))
                continue;

            vec2 probeBottomLeftPosition = probeCoordinate * mergeFromAngleResolution;
            probeBottomLeftPosition.x += mergeFromLeftPositionX;

            // Loop over every direction to sample in the from cascade
            for (int directionOffset = 0; directionOffset < 2; ++directionOffset)
            {
                int mergeFromAngleIndex = mergeToAngleIndex * 2 + directionOffset;

                ivec2 mergeFromAngleCoord = ivec2(
                    mergeFromAngleIndex % mergeFromAngleResolution.x,
                    floor(mergeFromAngleIndex / float(mergeFromAngleResolution.x)));

                vec2 samplePosition = probeBottomLeftPosition + mergeFromAngleCoord;
                vec4 fromRadiance = texture(cascadeTexture, samplePosition / cascadeTextureDimensions);
                radiance += fromRadiance * weights[yOffset * 2 + xOffset];
            }
        }
    }

    // Normalize radiance
    // Only dividing by 2 because the weights handles the 4x extra samples that occur for each direction
    radiance /= 2.0f;
    return mergeThing(toRadiance, radiance);
}

vec2 WorldPositionToProbeCoordinate(ivec2 probeResolution, vec2 position)
{
    // Divide by resolution + 2 so the grid sits away from the edges of the screen
    vec2 probeSpacing = worldTextureDimensions / (probeResolution + 1);

    // Subtract 1 here because otherwise it would be the coordinates would start flush from the top left corner
    return position / probeSpacing - 1;
}

ivec2 TransformThing(ivec2 probeCoordinate)
{
    return ivec2(floor((probeCoordinate - 1) * 0.5f));
}

vec2 ProbeCoordinateToWorldPosition(ivec2 probeResolution, ivec2 coords)
{
    // Divide by resolution + 1 so the grid sits away from the edges of the screen
    vec2 probeSpacing = worldTextureDimensions / (probeResolution + 1);

    // Add 1 here because otherwise it would be the coordinates would start flush from the top left corner
    return probeSpacing * (coords + 1);
}

void main(void)
{
    // Get pixel index
    ivec2 angleCoord = ivec2(toPixelCoordInput);
    angleCoord %= mergeToAngleResolution;

    int toAngleIndex = angleCoord.y * mergeToAngleResolution.x + angleCoord.x;
    
    // t does not reach 1.0f as angleIndex is 0 based while resolution is not
    // This is on purpose so we don't raycast 0 & 360 angle
    float angleT = toAngleIndex / float(mergeToAngleResolution.x * mergeToAngleResolution.y);
    
    // Adding 45 degrees so that e.g. the 4 raycasts are offset rather than on the axis aligned
    float rayAngle = angleT * (M_PI * 2.0f) + (M_PI * 0.25f);
    vec2 rayDirection = vec2(cos(rayAngle), sin(rayAngle));
    
    ivec2 toProbeCoord = ivec2(floor(toProbeCoordInput));
    ivec2 fromProbeCoord = TransformThing(toProbeCoord);// ivec2(floor(WorldPositionToProbeCoordinate(mergeFromProbeResolution, worldPosition)));

    vec2 probeCenterWorldPosition = ProbeCoordinateToWorldPosition(mergeToProbeResolution, toProbeCoord);

    vec4 finalRadiance = Raycast(probeCenterWorldPosition, rayDirection, mergeToIntervalMinMax.x, mergeToIntervalMinMax.y);

    // We don't need to waste samples merging if its the first one (ie nothing to merge), we just want to write the cast directly
    if (doMerge)
        finalRadiance = mergeIntervals(finalRadiance, toProbeCoord, ivec2(floor(fromProbeCoord)), toAngleIndex);
    
    color = finalRadiance;
}
