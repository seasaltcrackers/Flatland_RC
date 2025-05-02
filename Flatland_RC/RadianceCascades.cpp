#include <algorithm>
#include <imgui.h>

#include "RadianceCascades.h"
#include "Program.h"
#include "MeshGenerator.h"
#include "FrameBuffer.h"

void RadianceCascades::Initialise(int width, int height)
{
	Width = width;
	Height = height;

	AngularResolution = 8;
	ProbeResolution = 32;

	CascadeWidth = ProbeResolution * AngularResolution * 2;
	CascadeHeight = ProbeResolution * AngularResolution;

	CascadesFrameBuffer = new FrameBuffer(CascadeWidth, CascadeHeight);
	
	RenderProgram = Program::GenerateFromFileVsFs("Resources/Render.vs", "Resources/Render.fs");
	CascadeProgram = Program::GenerateFromFileVsFs("Resources/Cascade.vs", "Resources/Cascade.fs");

	FullscreenQuad = MeshGenerator::GenerateGrid({ 1, 1 }, { 2.0f, -2.0f }, { -1.0f, 1.0f });

	InitialiseBufferTexture();
	InitialiseCascadeTexture();
}

void RadianceCascades::Update()
{
	ImGui::Begin("Radiance Cascades");
	ImGui::Checkbox("Show Cascades", &ShowingCascadesTexture);
	ImGui::End();

	CascadesFrameBuffer->Bind();

	CascadeProgram->BindProgram();

	CascadeProgram->SetIVector("cascade0AngleResolution", glm::ivec2(AngularResolution, AngularResolution));
	CascadeProgram->SetIVector("cascade0ProbeResolution", glm::ivec2(ProbeResolution, ProbeResolution));
	CascadeProgram->SetVector("cascadeTextureDimensions", glm::vec2(CascadeWidth, CascadeHeight));

	CascadeProgram->SetTexture("worldTexture", TextureID);
	CascadeProgram->SetVector("worldTextureDimensions", glm::vec2{ Width, Height });

	FullscreenQuad->RenderMesh();

	CascadeProgram->UnbindProgram();

	CascadesFrameBuffer->Unbind();
}

void RadianceCascades::Render()
{
	RenderProgram->BindProgram();

	//RenderProgram->SetTexture("tex", TextureID);
	RenderProgram->SetTexture("tex", ShowingCascadesTexture ? CascadesFrameBuffer->GetTexture() : TextureID);
	FullscreenQuad->RenderMesh();

	RenderProgram->UnbindProgram();
}

void RadianceCascades::InitialiseBufferTexture()
{
	glGenTextures(1, &TextureID);
	glBindTexture(GL_TEXTURE_2D, TextureID);

	// Texture will clamp to the edge if sampled outside of it
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// Nearest pixel will be chosen (good for pixel art)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	// Generate texture data, empty black
	int size = Width * Height;
	Colour* data = new Colour[size];
	std::fill(data, data + size, Colour{ 0, 0, 0 });

	DrawRectangle(data, { 100, 100 }, { 200, 10 }, { 255, 255, 255 });

	// Set texture data
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, Width, Height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

	glBindTexture(GL_TEXTURE_2D, 0);
}

void RadianceCascades::InitialiseCascadeTexture()
{
}

void RadianceCascades::DrawRectangle(Colour* colourData, glm::ivec2 position, glm::ivec2 dimensions, Colour colour)
{
	for (int y = position.y; y < std::min(position.y + dimensions.y, Height - 1); ++y)
	{
		for (int x = position.x; x < std::min(position.x + dimensions.x, Width - 1); ++x)
			colourData[y * Width + x] = colour;
	}
}
