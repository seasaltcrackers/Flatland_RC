#include <iostream>
#include <gtc\type_ptr.hpp>

#include "FrameBuffer.h"
#include "Constants.h"

FrameBuffer::FrameBuffer(int width, int height, bool includeAlpha, bool bilinear)
{
	TextureWidth = width;
	TextureHeight = height;

	GLenum textureFormat = includeAlpha ? GL_RGBA : GL_RGB;
	GLenum textureType = GL_UNSIGNED_BYTE;
	GLenum attachmentType = GL_COLOR_ATTACHMENT0;

	// Setup the texture
	glGenTextures(1, &Texture);
	glBindTexture(GL_TEXTURE_2D, Texture);
	glTexImage2D(GL_TEXTURE_2D, 0, textureFormat, TextureWidth, TextureHeight, 0, textureFormat, textureType, NULL);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, bilinear ? GL_LINEAR : GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, bilinear ? GL_LINEAR : GL_NEAREST);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

	glBindTexture(GL_TEXTURE_2D, 0);

	// Setup the frame buffer
	glGenFramebuffers(1, &FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, attachmentType, GL_TEXTURE_2D, Texture, 0);

	//GLuint rbo;
	//glGenRenderbuffers(1, &rbo);
	//glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	//glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, TextureWidth, TextureHeight);
	//glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

	auto fboStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (fboStatus != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "Framebuffer not complete: " << fboStatus << std::endl;

	// Unbind
	//glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

FrameBuffer::~FrameBuffer()
{
}

void FrameBuffer::Bind(bool clear)
{
	glViewport(0, 0, TextureWidth, TextureHeight);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);

	if (clear)
		glClear(GL_COLOR_BUFFER_BIT);
}


void FrameBuffer::Unbind()
{
	glFlush();
	glFinish();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, Constants::WindowWidth, Constants::WindowHeight);
}

GLuint FrameBuffer::GetTexture()
{
	return Texture;
}