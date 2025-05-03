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
	Program* CascadeGenerateProgram = nullptr;
	Program* CascadeMergeProgram = nullptr;
	Mesh* FullscreenQuad = nullptr;

	GLuint TextureID = 0;

	int MaximumCascades = 0;

	float Cascade0IntervalLength= 0;
	glm::ivec2 Cascade0AngularResolution;
	glm::ivec2 Cascade0ProbeResolution;

	int Width = 0;
	int Height = 0;

	int CascadeWidth = 0;
	int CascadeHeight = 0;

	void InitialiseBufferTexture();
	void InitialiseCascadeTexture();

	glm::vec2 CalculateIntervalMinMax(int cascade);
	glm::ivec2 CalculateProbeResolution(int cascade);
	glm::ivec2 CalculateAngleResolution(int cascade);

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

