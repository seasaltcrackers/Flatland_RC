#pragma once
#include <glew.h>
#include <glm.hpp>

class FrameBuffer;
class Program;
class Mesh;

class RadianceCascades
{
public:

	void Initialise(int width, int height);

	void Update();
	void Render();

private:

	FrameBuffer* CascadesFrameBuffer = nullptr;

	Program* RenderProgram = nullptr;
	Program* CascadeProgram = nullptr;
	Mesh* FullscreenQuad = nullptr;

	GLuint TextureID = 0;

	int AngularResolution = 0;
	int ProbeResolution = 0;

	int Width = 0;
	int Height = 0;

	int CascadeWidth = 0;
	int CascadeHeight = 0;

	void InitialiseBufferTexture();
	void InitialiseCascadeTexture();


	struct Colour
	{
		GLubyte R;
		GLubyte G;
		GLubyte B;
	};

	void DrawRectangle(Colour* colourData, glm::ivec2 position, glm::ivec2 dimensions, Colour colour);

private:

	// IMGUI:

	bool ShowingCascadesTexture = true;

};

