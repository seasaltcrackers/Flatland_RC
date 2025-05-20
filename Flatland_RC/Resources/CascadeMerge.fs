#version 450 core

#define M_PI 3.1415926535897932384626433832795

in vec2 toProbeCoord;
in vec2 toPixelCoord;
in vec2 worldPosition;

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
uniform vec2 worldTextureDimensions;

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

    //return vec4(
    //    (1.0f - ratio.x),
    //    ratio.x,
    //    (1.0f - ratio.y),
    //    ratio.y
    //);
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
        //toProbeCoordinates 

        //vec2 toProbeInFromCoordSpace = (vec2(toProbeCoordinates * mergeToAngleResolution) * vec2(0.5f, 1.0f)) / mergeFromAngleResolution;
        //vec2 bilinearRatio = (fract(toProbeInFromCoordSpace) + vec2(0.25f, 0.25f));
        vec2 ratioThing = CalculateRatio(toProbeCoordinates);
        weights = bilinearWeights(ratioThing);
        

        //return vec4(ratioThing, 0.0f, 1.0f);
    }


    //if (bilinearFix)
    //{
    //    return vec4(weights.rgb, 1.0f);
    //}
    //else
    //{
    //    return vec4(bilinearRatio, 0.0f, 1.0f);
    //}

    //return vec2(0.75f, 0.25f, 0.25f, 0.25f);
    
    //if (any(lessThan(fromProbeCoordinates, ivec2(0, 0))) || any(greaterThanEqual(fromProbeCoordinates + 1, mergeFromProbeResolution)))
    //    return vec4(1, 1, 1, 1);
        
    vec4 radiance = vec4(0.0f, 0.0f, 0.0f, 0.0f);

    // Loop over every probe to merge in the from cascade
    for (int yOffset = 0; yOffset < 2; ++yOffset)
    {
        for (int xOffset = 0; xOffset < 2; ++xOffset)
        {
            ivec2 probeOffset = ivec2(xOffset, yOffset);
            ivec2 probeCoordinate = (fromProbeCoordinates + probeOffset);
            
            if (any(lessThan(probeCoordinate, ivec2(0, 0))) || any(greaterThanEqual(probeCoordinate, mergeFromProbeResolution)))
                {
                //radiance += vec4(1, 1, 1, 1);
                continue;
                //return vec4(1, 1, 1, 1);
                }

            vec2 probeBottomLeftPosition = probeCoordinate * mergeFromAngleResolution;
            probeBottomLeftPosition.x += mergeFromLeftPositionX;

            //return vec4(probeBottomLeftPosition / cascadeTextureDimensions, 0, 1);

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

    //return vec4(0, 0, 0, 1);

    // Normalize
    radiance /= 2.0f;
    //return radiance;

    //radiance.a = ceil(radiance.a);
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

void main(void)
{
    // Get pixel index
    ivec2 angleCoord = ivec2(toPixelCoord);
    angleCoord %= mergeToAngleResolution;

    int toAngleIndex = angleCoord.y * mergeToAngleResolution.x + angleCoord.x;
    
    //color = vec4(vec2(floor(toProbeCoord)) / mergeToProbeResolution, 0.0f, 1.0f);
    //return;

    ivec2 fromProbeCoord = TransformThing(ivec2(floor(toProbeCoord)));// ivec2(floor(WorldPositionToProbeCoordinate(mergeFromProbeResolution, worldPosition)));

    vec4 near = texture(cascadeTexture, toPixelCoord / cascadeTextureDimensions);
    vec4 far = mergeIntervals(near, ivec2(floor(toProbeCoord)), ivec2(floor(fromProbeCoord)), toAngleIndex);
    
    color = far;

    //color = far.a != 0.0f ? vec4(1.0f, 1.0f, 1.0f, 1.0f) : vec4(0.0f, 0.0f, 0.0f, 0.0f);
    //color = any(greaterThan(far, vec4(1.0f, 1.0f, 1.0f, 1.0f))) ? vec4(1.0f, 1.0f, 1.0f, 1.0f) : vec4(0.0f, 0.0f, 0.0f, 0.0f);

    //vec2 thing = ivec2(floor(toProbeCoord));
    //bool inRange = any(lessThan(thing, ivec2(0, 0))) || any(greaterThanEqual(thing, mergeToProbeResolution));
    //color = !inRange ? vec4(thing / mergeToProbeResolution, 0, 1) : vec4(1, 1, 1, 1);

    //vec2 thing = ivec2(floor(fromProbeCoord));
    //vec2 thing1 = ivec2(floor(fromProbeCoord + 1));
    //bool inRange = any(lessThan(thing, ivec2(0, 0))) || any(greaterThanEqual(thing1, mergeFromProbeResolution));
    //inRange = false;
    //color = !inRange ? vec4(0, 0, 0, 1) : vec4(1, 1, 1, 1);
}
