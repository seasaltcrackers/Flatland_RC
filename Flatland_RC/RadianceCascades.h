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
	FrameBuffer* WorldFrameBuffer = nullptr;

	Program* RenderProgram = nullptr;
	Program* RenderFullscreenProgram = nullptr;
	Program* CascadeRenderProgram = nullptr;
	Program* CascadeGenerateProgram = nullptr;
	Program* CascadeMergeProgram = nullptr;
	Mesh* FullscreenQuad = nullptr;

	//GLuint TextureID = 0;

	int MaximumCascades = 0;

	float Cascade0IntervalLength= 0;
	glm::ivec2 Cascade0AngularResolution;
	glm::ivec2 Cascade0ProbeResolution;

	int Width = 0;
	int Height = 0;

	int CascadeWidth = 0;
	int CascadeHeight = 0;

	glm::vec2 SquareAPosition = glm::vec2(293.0f, 66.0f);
	glm::vec2 SquareBPosition = glm::vec2(293.0f, 400.0f);

	glm::vec2 CalculateIntervalMinMax(int cascade);
	glm::ivec2 CalculateProbeResolution(int cascade);
	glm::ivec2 CalculateAngleResolution(int cascade);

	struct Colour
	{
		GLubyte R;
		GLubyte G;
		GLubyte B;
		GLubyte A;
	};

	void DrawRectangle(Colour* colourData, glm::ivec2 position, glm::ivec2 dimensions, Colour colour);

private:

	// IMGUI:

	int CurrentStage = 0;
	bool OutputBilinearFix = true;
	bool MergeBilinearFix = true;

};

