#version 450 core

layout (location = 0) in vec2 position;
layout (location = 1) in vec2 texCoord;

out vec2 fragTexCoord;
out vec2 fragPixelCoord;

uniform vec2 worldTextureDimensions;

void main()
{
	gl_Position = vec4(position, 0.0f, 1.0f);
	fragTexCoord = texCoord;
    fragPixelCoord = texCoord * worldTextureDimensions;
}