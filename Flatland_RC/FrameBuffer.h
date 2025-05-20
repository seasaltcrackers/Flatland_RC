#pragma once
#include <glew.h>
#include <glm.hpp>

class FrameBuffer
{
public:
	
	FrameBuffer(int width, int height, bool includeAlpha, bool bilinear);
	~FrameBuffer();

public:

	void Bind(bool clear = true);
	void Unbind();

	GLuint GetTexture();

private:

	GLuint Texture;
	GLuint FBO;

	GLsizei TextureWidth;
	GLsizei TextureHeight;

};