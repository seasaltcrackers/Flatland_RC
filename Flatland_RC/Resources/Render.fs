#version 450 core

in vec2 fragTexCoord;

out vec4 color;

uniform vec4 colour;

void main(void)
{
	color = colour;
}