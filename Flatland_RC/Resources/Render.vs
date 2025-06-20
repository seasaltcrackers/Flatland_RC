#version 450 core

layout (location = 0) in vec2 position;
layout (location = 1) in vec2 texCoord;

out vec2 fragTexCoord;

uniform mat4 PVM;

void main()
{
	gl_Position = PVM * vec4(position, 0.0f, 1.0f);
	fragTexCoord = texCoord;
}