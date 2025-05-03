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

	MaximumCascades = 6;
	Cascade0IntervalLength = 4.0f;
	Cascade0AngularResolution = glm::ivec2(8, 8);
	Cascade0ProbeResolution = glm::ivec2(32, 32);

	CascadeWidth = Cascade0ProbeResolution.x * Cascade0AngularResolution.x * 2;
	CascadeHeight = Cascade0ProbeResolution.y * Cascade0AngularResolution.y;

	CascadesFrameBuffer = new FrameBuffer(CascadeWidth, CascadeHeight);
	
	RenderProgram = Program::GenerateFromFileVsFs("Resources/Render.vs", "Resources/Render.fs");
	CascadeGenerateProgram = Program::GenerateFromFileVsFs("Resources/CascadeGenerate.vs", "Resources/CascadeGenerate.fs");
	CascadeMergeProgram = Program::GenerateFromFileVsFs("Resources/CascadeMerge.vs", "Resources/CascadeMerge.fs");

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

	CascadeGenerateProgram->BindProgram();

	CascadeGenerateProgram->SetIVector("cascade0AngleResolution", Cascade0AngularResolution);
	CascadeGenerateProgram->SetIVector("cascade0ProbeResolution", Cascade0ProbeResolution);
	CascadeGenerateProgram->SetVector("cascadeTextureDimensions", glm::vec2(CascadeWidth, CascadeHeight));

	CascadeGenerateProgram->SetTexture("worldTexture", TextureID);
	CascadeGenerateProgram->SetVector("worldTextureDimensions", glm::vec2{ Width, Height });

	FullscreenQuad->RenderMesh();

	CascadeGenerateProgram->UnbindProgram();

	CascadeMergeProgram->BindProgram();

	CascadeMergeProgram->SetTexture("cascadeTexture", CascadesFrameBuffer->GetTexture());
	CascadeMergeProgram->SetVector("cascadeTextureDimensions", glm::vec2(CascadeWidth, CascadeHeight));

	for (int cascade = MaximumCascades - 2; cascade >= 0; --cascade)
	{
		int mergeFromCascade = cascade + 1;
		int mergeToCascade = cascade;

		float currentXOffset = 1.0f - std::powf(0.5f, mergeToCascade);
		float nextXOffset = 1.0f - std::powf(0.5f, mergeFromCascade);
		float xScale = nextXOffset - currentXOffset;

		CascadeMergeProgram->SetIVector("mergeFromProbeResolution", CalculateProbeResolution(mergeFromCascade));
		CascadeMergeProgram->SetIVector("mergeToProbeResolution", CalculateProbeResolution(mergeToCascade));

		CascadeMergeProgram->SetIVector("mergeFromAngleResolution", CalculateAngleResolution(mergeFromCascade));
		CascadeMergeProgram->SetIVector("mergeToAngleResolution", CalculateAngleResolution(mergeToCascade));

		CascadeMergeProgram->SetInt("mergeFromCascade", mergeFromCascade);
		CascadeMergeProgram->SetInt("mergeToCascade", mergeToCascade);

		CascadeMergeProgram->SetVector("horizontalTransform", glm::vec2(currentXOffset, xScale));

		FullscreenQuad->RenderMesh();
	}

	CascadeMergeProgram->UnbindProgram();
	CascadesFrameBuffer->Unbind();
}

void RadianceCascades::Render()
{
	RenderProgram->BindProgram();

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

glm::vec2 RadianceCascades::CalculateIntervalMinMax(int cascade)
{
    float intervalLength = Cascade0IntervalLength * pow(2.0f, cascade);
    float intervalOffset = intervalLength * (2.0f / 3.0f);
    return glm::vec2(intervalOffset, intervalOffset + intervalLength);
}

glm::ivec2 RadianceCascades::CalculateProbeResolution(int cascade)
{
    return glm::ivec2(
        std::ceilf(Cascade0ProbeResolution.x / std::powf(2.0f, cascade)),
		std::ceilf(Cascade0ProbeResolution.y / std::powf(2.0f, cascade)));
}

glm::ivec2 RadianceCascades::CalculateAngleResolution(int cascade)
{
    return glm::ivec2(
		Cascade0AngularResolution.x,
		Cascade0AngularResolution.y * std::pow(2, cascade));
}
