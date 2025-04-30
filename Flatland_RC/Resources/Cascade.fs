#version 450 core

//in int index;
in vec2 fragTexCoord;
in vec2 fragPixelCoord;

out vec4 color;

uniform sampler2D worldTexture;
uniform vec2 worldTextureDimensions;

void main(void)
{
    int resolution = 50;

    vec4 worldColour = texture(worldTexture, fragTexCoord);

    ivec2 pixelCoord = ivec2(fragPixelCoord);
    pixelCoord %= ivec2(resolution, resolution);

    int index = pixelCoord.y * resolution + pixelCoord.x;

    float greyscale = index * (255.0f / (resolution * resolution));
    color = vec4(pixelCoord.xy, 0, worldColour.a);

	//color = vec4(fragTexCoord.xy, 0, 1);
	//color = vec4(fragPixelCoord / worldTextureDimensions, 0, worldColour.a);
	//color = vec4(worldTextureDimensions, 0, 1);
}

void Raycast(vec2 position, vec2 direction, float intervalMin, float intervalMax)
{
    
}