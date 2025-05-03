#version 450 core

#define M_PI 3.1415926535897932384626433832795

in vec2 fromProbeCoord;
in vec2 toProbeCoord;

in vec2 fromPixelCoord;
in vec2 toPixelCoord;

out vec4 color;

uniform int mergeToCascade;

uniform float cascade0IntervalLength;
uniform ivec2 cascade0ProbeResolution;
uniform ivec2 cascade0AngleResolution;

uniform sampler2D worldTexture;
uniform vec2 worldTextureDimensions;

uniform sampler2D cascadeTexture;
uniform vec2 cascadeTextureDimensions;

//vec3 Raycast(vec2 rayOrigin, vec2 rayDirection, float intervalMin, float intervalMax)
//{
//    for (float rayDistance = intervalMin; rayDistance <= intervalMax; rayDistance += 1.0f)
//    {
//        vec2 samplePosition = rayOrigin + rayDirection * rayDistance;
//        vec4 color = texture(worldTexture, samplePosition / worldTextureDimensions);
//
//        if (color.a == 1.0f)
//            return color.rgb;
//    }
//
//    return vec3(0.0f, 0.0f, 0.0f);
//}
//
//vec2 CalculateIntervalMinMax(int cascade)
//{
//    float intervalLength = cascade0IntervalLength * pow(2.0f, cascade);
//    float intervalOffset = intervalLength * (2.0f / 3.0f);
//    return vec2(intervalOffset, intervalOffset + intervalLength);
//}
//
//ivec2 CalculateProbeResolution(int cascade)
//{
//    return ivec2(
//        ceil(cascade0ProbeResolution.x / pow(2.0f, cascade)),
//        ceil(cascade0ProbeResolution.y / pow(2.0f, cascade)));
//}
//
//ivec2 CalculateAngleResolution(int cascade)
//{
//    return ivec2(
//        cascade0AngleResolution.x,
//        cascade0AngleResolution.y * pow(2, cascade));
//}
//
//vec2 ProbeCoordinateToWorldPosition(ivec2 probeResolution, ivec2 coords)
//{
//    // Divide by resolution + 1 so the grid sits away from the edges of the screen
//    vec2 probeSpacing = worldTextureDimensions / (probeResolution + 1);
//
//    // Add 1 here because otherwise it would be the coordinates would start flush from the top left corner
//    return probeSpacing * (coords + 1);
//}
//
//ivec2 CascadePositionToProbeCoordinate(int cascade, ivec2 angularResolution, vec2 position)
//{
//    float cascadeStartXNormalized = 1.0f - pow(0.5f, cascade);
//    position.x -= (cascadeStartXNormalized * cascadeTextureDimensions.x);
//    return ivec2(floor(position / angularResolution));
//}

void main(void)
{
    // Get current origin position & raycast direction

    // Get nearest cascade + 1 probe relavent directions

    // Merge into current raycast direction
    color = vec4(mergeToCascade.rrr / 10.0f, 1.0f);


    // We need to find the probe coordinate of the current pixel

    // We need to find the 4 closest probes on cascade + 1



    //CascadePositionToProbeCoordinate();


    // Get the cascade index of the current pixel based on how far across horizontally it is
    // Assumes width is 2 * cascade 0 width
    //int cascade = int(floor(log(1.0f - fragTexCoord.x) / log(0.5f)));
    //
    //ivec2 probeResolution = CalculateProbeResolution(cascade);
    //ivec2 angularResolution = CalculateAngleResolution(cascade);
    //vec2 intervalMinMax = CalculateIntervalMinMax(cascade);
    //
    //ivec2 angleCoord = ivec2(fragPixelCoord);
    //angleCoord %= angularResolution;
    //
    //int angleIndex = angleCoord.y * angularResolution.x + angleCoord.x;
    //
    //// t does not reach 1.0f as angleIndex is 0 based while resolution is not
    //// This is on purpose so we don't raycast 0 & 360 angle
    //float angleT = angleIndex / float(angularResolution.x * angularResolution.y);
    //
    //// Adding 45 degrees so that e.g. the 4 raycasts are offset rather than on the axis aligned
    //float rayAngle = angleT * (M_PI * 2.0f) + (M_PI * 0.25f);
    //vec2 rayDirection = vec2(cos(rayAngle), sin(rayAngle));
    //
    //ivec2 probeCoordinate = CascadePositionToProbeCoordinate(cascade, angularResolution, fragPixelCoord);
    //vec2 probeCenterWorldPosition = ProbeCoordinateToWorldPosition(probeResolution, probeCoordinate);
    //
    //// Raycast in the direction 
    //vec3 radiance = Raycast(probeCenterWorldPosition, rayDirection, intervalMinMax.x, intervalMinMax.y);
    //color = vec4(radiance, 1.0f);
}
