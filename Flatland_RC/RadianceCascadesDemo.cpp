#include <algorithm>
#include <imgui.h>
#include <gtc/matrix_transform.hpp>
#include <gtx/string_cast.hpp>

#include "RadianceCascadesDemo.h"
#include "Program.h"
#include "MeshGenerator.h"
#include "FrameBuffer.h"
#include "Input.h"

void RadianceCascadesDemo::Initialise(int width, int height, glm::ivec2 cascade0AngularResolution, int cascade0ProbeSpacing)
{
	Width = width;
	Height = height;

	// Calculate the dimensions to the nearest power of 2 this is to ensure probes resolution can scale down correctly by 2x with each cascade while still allowing for arbitrary canvas sizes
	glm::vec2 dimensionExponents = glm::log2(glm::vec2(width, height));
	glm::ivec2 dimensionExponentsRounded = glm::round(dimensionExponents);
	glm::ivec2 dimensionsToNearestPower = glm::ivec2(std::pow(2, dimensionExponentsRounded.x), std::pow(2, dimensionExponentsRounded.y));

	glm::mat4 p = glm::ortho(0.0f, (float)Width, 0.0f, (float)Height, -0.1f, 1000.0f);
	glm::mat4 v = glm::lookAt(glm::vec3(0.0f, 0.0f, 10.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	ProjectionView = p * v;

	MaximumCascades = 10;
	Cascade0IntervalLength = 5.0f;
	Cascade0AngularResolution = cascade0AngularResolution;
	Cascade0ProbeResolution = dimensionsToNearestPower / cascade0ProbeSpacing; // Attempt to go for 1 probe per 2 pixels as close as possible
	
	CascadeWidth = Cascade0ProbeResolution.x * Cascade0AngularResolution.x * 2;
	CascadeHeight = Cascade0ProbeResolution.y * Cascade0AngularResolution.y;

	BaseWorldFrameBuffer = new FrameBuffer(Width, Height, true, false);
	FinalWorldFrameBuffer = new FrameBuffer(Width, Height, true, false);
	CascadesFrameBuffer = new FrameBuffer(CascadeWidth, CascadeHeight, true, false);

	RenderProgram = Program::GenerateFromFileVsFs("Resources/Render.vs", "Resources/Render.fs");
	RenderFullscreenProgram = Program::GenerateFromFileVsFs("Resources/RenderFullscreen.vs", "Resources/RenderFullscreen.fs");
	CascadeRenderProgram = Program::GenerateFromFileVsFs("Resources/CascadeRender.vs", "Resources/CascadeRender.fs");
	CascadeMergeProgram = Program::GenerateFromFileVsFs("Resources/CascadeMerge.vs", "Resources/CascadeMerge.fs");

	FullscreenQuad = MeshGenerator::GenerateGrid({ 1, 1 }, { 2.0f, -2.0f }, { -1.0f, 1.0f });

	PaintBrushColour[0] = 1.0f;
	PaintBrushColour[1] = 1.0f;
	PaintBrushColour[2] = 1.0f;

	PaintBrushDimensions[0] = 10.0f;
	PaintBrushDimensions[1] = 10.0f;
}

void RadianceCascadesDemo::Update()
{
	glClearColor(0.0, 0.0, 0.0, 0.0); // Transparent


	ImGui::Begin("Radiance Cascades");
	ImGui::SliderInt("Maximum Cascades", &MaximumCascades, 1, 10);
	ImGui::SliderFloat("Interval", &Cascade0IntervalLength, 0.1f, 100.0f);
	ImGui::Combo("Stage", &CurrentStage, "Final\0Cascades Merged\0World");
	ImGui::Combo("Sample", &SampleType, "Nearest\0Average\0Bilinear");
	ImGui::Checkbox("Enable Bilinear Fix", &EnableBilinearFix);
	ImGui::Checkbox("Enable sRGB", &OutputEnableSRGB);
	ImGui::Checkbox("Merge Bilinear Interpolate", &MergeBilinearInterpolation);
	ImGui::ColorPicker3("Drawing Colour", PaintBrushColour);

	ImGui::BeginGroup();

	ImGui::Checkbox("Square Paint Brush", &PaintBrushIsSquare);

	if (PaintBrushIsSquare)
	{
		ImGui::SliderFloat("Paint Brush Dimensions", PaintBrushDimensions, 5.0f, 300.0f);
		PaintBrushDimensions[1] = PaintBrushDimensions[0];
	}
	else
	{
		ImGui::SliderFloat2("Paint Brush Dimensions", PaintBrushDimensions, 5.0f, 300.0f);
	}

	ImGui::EndGroup();

	if (ImGui::Button("Clear Canvas"))
	{
		// Clear base world buffer
		BaseWorldFrameBuffer->Bind(true);
		BaseWorldFrameBuffer->Unbind();
	}

	ImGui::End();


	if (Input::IsMouseDown(0) || Input::IsMouseDown(1))
	{
		BaseWorldFrameBuffer->Bind(false);

		RenderPaintBrush();

		BaseWorldFrameBuffer->Unbind();

		glMemoryBarrier(GL_FRAMEBUFFER_BARRIER_BIT);
	}

	FinalWorldFrameBuffer->Bind();

	// Render Base:
	RenderFullscreenProgram->BindProgram();

	RenderFullscreenProgram->SetTexture("tex", BaseWorldFrameBuffer->GetTexture());
	FullscreenQuad->RenderMesh();

	RenderFullscreenProgram->UnbindProgram();

	// Render paint brush
	RenderPaintBrush();

	FinalWorldFrameBuffer->Unbind();

	glMemoryBarrier(GL_FRAMEBUFFER_BARRIER_BIT);

	CascadesFrameBuffer->Bind();

	CascadeMergeProgram->BindProgram();

	CascadeMergeProgram->SetTexture("worldTexture", FinalWorldFrameBuffer->GetTexture());
	CascadeMergeProgram->SetVector("worldTextureDimensions", glm::vec2{ Width, Height });

	CascadeMergeProgram->SetTexture("cascadeTexture", CascadesFrameBuffer->GetTexture());
	CascadeMergeProgram->SetVector("cascadeTextureDimensions", glm::vec2(CascadeWidth, CascadeHeight));
	
	CascadeMergeProgram->SetBool("enableBilinearFix", EnableBilinearFix);
	CascadeMergeProgram->SetBool("bilinearInterpolation", MergeBilinearInterpolation);
	CascadeMergeProgram->SetBool("doMerge", false);

	for (int cascade = MaximumCascades - 2; cascade >= 0; --cascade)
	{
		glMemoryBarrier(GL_FRAMEBUFFER_BARRIER_BIT);

		int mergeFromCascade = cascade + 1;
		int mergeToCascade = cascade;

		float xOffsetTo = 1.0f - std::powf(0.5f, mergeToCascade);
		float xOffsetFrom = 1.0f - std::powf(0.5f, mergeFromCascade);

		float toXScale = xOffsetFrom - xOffsetTo;

		CascadeMergeProgram->SetVector("mergeToIntervalMinMax", CalculateIntervalMinMax(mergeToCascade));

		CascadeMergeProgram->SetFloat("mergeFromLeftPositionX", xOffsetFrom * CascadeWidth);

		CascadeMergeProgram->SetIVector("mergeFromProbeResolution", CalculateProbeResolution(mergeFromCascade));
		CascadeMergeProgram->SetIVector("mergeToProbeResolution", CalculateProbeResolution(mergeToCascade));

		CascadeMergeProgram->SetIVector("mergeFromAngleResolution", CalculateAngleResolution(mergeFromCascade));
		CascadeMergeProgram->SetIVector("mergeToAngleResolution", CalculateAngleResolution(mergeToCascade));

		CascadeMergeProgram->SetVector("toHorizontalTransform", glm::vec2(xOffsetTo, toXScale));

		FullscreenQuad->RenderMesh();

		// We don't merge the last cascade but merge all subsequent ones
		// This is because the last cascade has nothing to merge from
		if (cascade == MaximumCascades - 2)
			CascadeMergeProgram->SetBool("doMerge", true);
	}

	CascadeMergeProgram->UnbindProgram();
	CascadesFrameBuffer->Unbind();

	glClearColor(1.0, 0.0, 0.0, 1.0); // Red
}

void RadianceCascadesDemo::Render()
{
	// Final
	if (CurrentStage == 0)
	{
		glViewport(0, 0, Width, Height);

		CascadeRenderProgram->BindProgram();

		CascadeRenderProgram->SetInt("sampleType", SampleType);
		CascadeRenderProgram->SetBool("enableSRGB", OutputEnableSRGB);

		CascadeRenderProgram->SetIVector("cascade0AngleResolution", Cascade0AngularResolution);
		CascadeRenderProgram->SetIVector("cascade0ProbeResolution", Cascade0ProbeResolution);

		CascadeRenderProgram->SetVector("worldTextureDimensions", glm::vec2{ Width, Height });

		CascadeRenderProgram->SetTexture("cascadeTexture", CascadesFrameBuffer->GetTexture());
		CascadeRenderProgram->SetVector("cascadeTextureDimensions", glm::vec2(CascadeWidth, CascadeHeight));

		FullscreenQuad->RenderMesh();

		CascadeRenderProgram->UnbindProgram();
	}
	// Cascades
	else if (CurrentStage == 1)
	{
		glViewport(0, 0, CascadeWidth, CascadeHeight);

		RenderFullscreenProgram->BindProgram();

		RenderFullscreenProgram->SetTexture("tex", CascadesFrameBuffer->GetTexture());
		FullscreenQuad->RenderMesh();

		RenderFullscreenProgram->UnbindProgram();
	}
	// World
	else if (CurrentStage == 2)
	{
		glViewport(0, 0, Width, Height);

		RenderFullscreenProgram->BindProgram();

		RenderFullscreenProgram->SetTexture("tex", FinalWorldFrameBuffer->GetTexture());
		FullscreenQuad->RenderMesh();

		RenderFullscreenProgram->UnbindProgram();
	}
}

void RadianceCascadesDemo::RenderPaintBrush()
{
	RenderProgram->BindProgram();

	glm::vec2 position = Input::GetMousePosition();

	glm::mat4 model = glm::mat4();
	model = glm::translate(model, glm::vec3(position, 0.0f));
	model = glm::scale(model, glm::vec3(PaintBrushDimensions[0], PaintBrushDimensions[1], 1.0f));
	glm::mat4 pvm = ProjectionView * model;

	glm::vec4 rgba = glm::vec4(PaintBrushColour[0], PaintBrushColour[1], PaintBrushColour[2], 1.0f);

	// Erase on right click
	if (Input::IsMouseDown(1))
		rgba = glm::vec4(0, 0, 0, 0);

	RenderProgram->SetMatrix("PVM", pvm);
	RenderProgram->SetVector("colour", rgba);

	FullscreenQuad->RenderMesh();

	RenderProgram->UnbindProgram();
}

float RadianceCascadesDemo::CalculateIntervalScale(int cascade)
{
	if (cascade <= 0)
		return 0.0;

	// Scale interval by 2x each cascade
	return float(1 << (cascade));
}

glm::vec2 RadianceCascadesDemo::CalculateIntervalMinMax(int cascade)
{
	return Cascade0IntervalLength * glm::vec2(CalculateIntervalScale(cascade), CalculateIntervalScale(cascade + 1));
}

glm::ivec2 RadianceCascadesDemo::CalculateProbeResolution(int cascade)
{
	return glm::ivec2(
		std::ceilf(Cascade0ProbeResolution.x / std::powf(2.0f, cascade)),
		std::ceilf(Cascade0ProbeResolution.y / std::powf(2.0f, cascade)));
}

glm::ivec2 RadianceCascadesDemo::CalculateAngleResolution(int cascade)
{
	return glm::ivec2(
		Cascade0AngularResolution.x,
		Cascade0AngularResolution.y * std::pow(2, cascade));
}
