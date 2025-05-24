#include <algorithm>
#include <imgui.h>
#include <gtc/matrix_transform.hpp>
#include <gtx/string_cast.hpp>

#include "RadianceCascades.h"
#include "Program.h"
#include "MeshGenerator.h"
#include "FrameBuffer.h"
#include "Input.h"

void RadianceCascades::Initialise(int width, int height)
{
	Width = width;
	Height = height;

	glm::mat4 p = glm::ortho(0.0f, (float)Width, 0.0f, (float)Height, -0.1f, 1000.0f);
	glm::mat4 v = glm::lookAt(glm::vec3(0.0f, 0.0f, 10.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	ProjectionView = p * v;

	MaximumCascades = 10;
	Cascade0IntervalLength = 10.0f;
	Cascade0AngularResolution = glm::ivec2(4, 4);
	Cascade0ProbeResolution = glm::ivec2(Width, Height) / 2;

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

	PaintBrushDimensions[0] = 20.0f;
	PaintBrushDimensions[1] = 20.0f;
}

void RadianceCascades::Update()
{
	glClearColor(0.0, 0.0, 0.0, 0.0); // Transparent


	ImGui::Begin("Radiance Cascades");
	ImGui::SliderInt("Maximum Cascades", &MaximumCascades, 1, 16);
	ImGui::SliderFloat("Interval", &Cascade0IntervalLength, 0.1f, 100.0f);
	ImGui::Combo("Stage", &CurrentStage, "Final\0Cascades Merged\0Cascades\0World");
	ImGui::Combo("Sample", &SampleType, "Nearest\0Average\0Bilinear");
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

		glMemoryBarrier(GL_ALL_BARRIER_BITS);
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

	glMemoryBarrier(GL_ALL_BARRIER_BITS);

	CascadesFrameBuffer->Bind();

	if (CurrentStage == 2)
	{
		CascadesFrameBuffer->Unbind();
		glClearColor(1.0, 0.0, 0.0, 1.0); // Red
		return;
	}

	CascadeMergeProgram->BindProgram();

	CascadeMergeProgram->SetTexture("worldTexture", FinalWorldFrameBuffer->GetTexture());
	CascadeMergeProgram->SetVector("worldTextureDimensions", glm::vec2{ Width, Height });

	CascadeMergeProgram->SetTexture("cascadeTexture", CascadesFrameBuffer->GetTexture());
	CascadeMergeProgram->SetVector("cascadeTextureDimensions", glm::vec2(CascadeWidth, CascadeHeight));
	
	CascadeMergeProgram->SetBool("bilinearInterpolation", MergeBilinearInterpolation);
	CascadeMergeProgram->SetBool("doMerge", false);

	for (int cascade = MaximumCascades - 2; cascade >= 0; --cascade)
	{
		glMemoryBarrier(GL_ALL_BARRIER_BITS);

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

	glMemoryBarrier(GL_ALL_BARRIER_BITS);

	glClearColor(1.0, 0.0, 0.0, 1.0); // Red
}

void RadianceCascades::Render()
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
	else if (CurrentStage == 1 || CurrentStage == 2)
	{
		glViewport(0, 0, CascadeWidth, CascadeHeight);

		RenderFullscreenProgram->BindProgram();

		RenderFullscreenProgram->SetTexture("tex", CascadesFrameBuffer->GetTexture());
		FullscreenQuad->RenderMesh();

		RenderFullscreenProgram->UnbindProgram();
	}
	// World
	else if (CurrentStage == 3)
	{
		glViewport(0, 0, Width, Height);

		RenderFullscreenProgram->BindProgram();

		RenderFullscreenProgram->SetTexture("tex", FinalWorldFrameBuffer->GetTexture());
		FullscreenQuad->RenderMesh();

		RenderFullscreenProgram->UnbindProgram();
	}
}

void RadianceCascades::RenderPaintBrush()
{
	RenderProgram->BindProgram();

	glm::vec2 position = Input::GetMousePosition();

	glm::mat4 model = glm::mat4();
	model = glm::translate(model, glm::vec3(position, 0.0f));
	model = glm::scale(model, glm::vec3(PaintBrushDimensions[0], PaintBrushDimensions[1], 1.0f));

	glm::mat4 pvm = ProjectionView * model;

	//double totalTime = glfwGetTime();
	//glm::vec3 rgb = HsvToRgb({ std::fmodf(totalTime * 10.0f, 360.0f), 1.0f, 1.0f });

	glm::vec4 rgba = glm::vec4(PaintBrushColour[0], PaintBrushColour[1], PaintBrushColour[2], 1.0f);

	if (Input::IsMouseDown(1))
		rgba = glm::vec4(0, 0, 0, 0);

	RenderProgram->SetMatrix("PVM", pvm);
	RenderProgram->SetVector("colour", rgba);

	FullscreenQuad->RenderMesh();

	RenderProgram->UnbindProgram();
}

float RadianceCascades::CalculateIntervalScale(int cascade)
{
	if (cascade <= 0)
		return 0.0;

	/* Scale interval by 2x each cascade */
	return float(1 << (cascade));
}

glm::vec2 RadianceCascades::CalculateIntervalMinMax(int cascade)
{
	return Cascade0IntervalLength * glm::vec2(CalculateIntervalScale(cascade), CalculateIntervalScale(cascade + 1));
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
