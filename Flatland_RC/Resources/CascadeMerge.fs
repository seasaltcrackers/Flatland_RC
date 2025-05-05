#version 450 core

#define M_PI 3.1415926535897932384626433832795

in vec2 fromProbeCoord;
in vec2 toProbeCoord;

in vec2 fromPixelCoord;
in vec2 toPixelCoord;

out vec4 color;

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

vec4 bilinearWeights(vec2 ratio) {
    return vec4(
        (1.0f - ratio.x) * (1.0f - ratio.y),
        ratio.x * (1.0f - ratio.y),
        (1.0f - ratio.x) * ratio.y,
        ratio.x * ratio.y
    );
}

vec4 mergeIntervals(vec4 toRadiance, ivec2 toProbeCoordinates, ivec2 fromProbeCoordinates, int mergeToAngleIndex)
{
    vec2 toProbeInFromCoordSpace = (vec2(toProbeCoordinates * mergeToAngleResolution) * vec2(0.5f, 1.0f)) / mergeFromAngleResolution;
    vec2 bilinearRatio = fract(toProbeInFromCoordSpace);
    vec4 weights = bilinearWeights(bilinearRatio);
    
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
            for (int directionOffset = 0; directionOffset < 2; ++directionOffset)
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
    // Get current origin position & raycast direction

    // Get nearest cascade + 1 probe relavent directions

    // Merge into current raycast direction

    //color = vec4(floor(fromProbeCoord) / mergeFromProbeResolution, 0.0f, 1.0f);
    //color = vec4(mergeToCascade.rrr / 10.0f, 1.0f);

    // Get pixel index
    ivec2 angleCoord = ivec2(toPixelCoord);
    angleCoord %= mergeToAngleResolution;

    int toAngleIndex = angleCoord.y * mergeToAngleResolution.x + angleCoord.x;
    
    vec4 near = texture(cascadeTexture, toPixelCoord / cascadeTextureDimensions);
    vec4 far = mergeIntervals(near, ivec2(floor(toProbeCoord)), ivec2(floor(fromProbeCoord)), toAngleIndex);
    
    //color = mergeThing(near, far);

    color = far;
    //color = vec4((toAngleIndex / float(mergeToAngleResolution.x * mergeToAngleResolution.y)).rrr, 1.0f);

    //color = vec4(floor(fromProbeCoord) / mergeFromProbeResolution, 0.0f, 1.0f);

    //color = mix(vec4(vec2(angleCoord) / mergeToAngleResolution, 0.0f, 1.0f), color, color.a);
    //color = vec4(vec2(angleCoord) / mergeToAngleResolution, 0.0f, 1.0f);

    //color = vec4(vec2(angleCoord) / mergeToAngleResolution, 0.0f, 1.0f);

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
