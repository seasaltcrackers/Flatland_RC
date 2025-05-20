#include <algorithm>
#include <imgui.h>
#include <gtc/matrix_transform.hpp>

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
	Cascade0IntervalLength = 5.0f;
	Cascade0AngularResolution = glm::ivec2(4, 4);
	Cascade0ProbeResolution = glm::ivec2(width, height) / 2;

	CascadeWidth = Cascade0ProbeResolution.x * Cascade0AngularResolution.x * 2;
	CascadeHeight = Cascade0ProbeResolution.y * Cascade0AngularResolution.y;

	BaseWorldFrameBuffer = new FrameBuffer(Width, Height, true, false);
	FinalWorldFrameBuffer = new FrameBuffer(Width, Height, true, false);
	CascadesFrameBuffer = new FrameBuffer(CascadeWidth, CascadeHeight, true, false);

	RenderProgram = Program::GenerateFromFileVsFs("Resources/Render.vs", "Resources/Render.fs");
	RenderFullscreenProgram = Program::GenerateFromFileVsFs("Resources/RenderFullscreen.vs", "Resources/RenderFullscreen.fs");
	CascadeRenderProgram = Program::GenerateFromFileVsFs("Resources/CascadeRender.vs", "Resources/CascadeRender.fs");
	CascadeGenerateProgram = Program::GenerateFromFileVsFs("Resources/CascadeGenerate.vs", "Resources/CascadeGenerate.fs");
	CascadeMergeProgram = Program::GenerateFromFileVsFs("Resources/CascadeMerge.vs", "Resources/CascadeMerge.fs");

	FullscreenQuad = MeshGenerator::GenerateGrid({ 1, 1 }, { 2.0f, -2.0f }, { -1.0f, 1.0f });
}

void RadianceCascades::Update()
{
	ImGui::Begin("Radiance Cascades");
	ImGui::SliderInt("Maximum Cascades", &MaximumCascades, 1, 16);
	ImGui::SliderFloat("Interval", &Cascade0IntervalLength, 0.1f, 100.0f);
	ImGui::Combo("Stage", &CurrentStage, "Final\0Cascades Merged\0Cascades\0World");
	ImGui::Checkbox("Output Bilinear Interpolate", &OutputBilinearFix);
	ImGui::Checkbox("Merge Bilinear Interpolate", &MergeBilinearFix);
	ImGui::End();

	glClearColor(0.0, 0.0, 0.0, 0.0); // Black

	//BaseWorldFrameBuffer = new FrameBuffer(Width, Height, true, false);
	//FinalWorldFrameBuffer = new FrameBuffer(Width, Height, true, false);

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

	CascadeGenerateProgram->BindProgram();

	CascadeGenerateProgram->SetFloat("cascade0IntervalLength", Cascade0IntervalLength);
	CascadeGenerateProgram->SetIVector("cascade0AngleResolution", Cascade0AngularResolution);
	CascadeGenerateProgram->SetIVector("cascade0ProbeResolution", Cascade0ProbeResolution);
	CascadeGenerateProgram->SetVector("cascadeTextureDimensions", glm::vec2(CascadeWidth, CascadeHeight));

	CascadeGenerateProgram->SetTexture("worldTexture", FinalWorldFrameBuffer->GetTexture());
	CascadeGenerateProgram->SetVector("worldTextureDimensions", glm::vec2{ Width, Height });

	FullscreenQuad->RenderMesh();

	CascadeGenerateProgram->UnbindProgram();

	if (CurrentStage == 2)
	{
		CascadesFrameBuffer->Unbind();
		glClearColor(1.0, 0.0, 0.0, 1.0); // Red
		return;
	}

	CascadeMergeProgram->BindProgram();

	CascadeMergeProgram->SetTexture("cascadeTexture", CascadesFrameBuffer->GetTexture());
	CascadeMergeProgram->SetVector("cascadeTextureDimensions", glm::vec2(CascadeWidth, CascadeHeight));
	CascadeMergeProgram->SetVector("worldTextureDimensions", glm::vec2{ Width, Height });
	CascadeMergeProgram->SetBool("bilinearFix", MergeBilinearFix);

	for (int cascade = MaximumCascades - 2; cascade >= 0; --cascade)
	{
		glMemoryBarrier(GL_ALL_BARRIER_BITS);

		int mergeFromCascade = cascade + 1;
		int mergeToCascade = cascade;

		float xOffset1 = 1.0f - std::powf(0.5f, mergeToCascade);
		float xOffset2 = 1.0f - std::powf(0.5f, mergeFromCascade);
		float xOffset3 = 1.0f - std::powf(0.5f, mergeFromCascade + 1);

		float toXScale = xOffset2 - xOffset1;
		float fromXScale = xOffset3 - xOffset2;

		CascadeMergeProgram->SetFloat("mergeFromLeftPositionX", xOffset2 * CascadeWidth);
		CascadeMergeProgram->SetFloat("mergeToLeftPositionX", xOffset1 * CascadeWidth);

		CascadeMergeProgram->SetIVector("mergeFromProbeResolution", CalculateProbeResolution(mergeFromCascade));
		CascadeMergeProgram->SetIVector("mergeToProbeResolution", CalculateProbeResolution(mergeToCascade));

		CascadeMergeProgram->SetIVector("mergeFromAngleResolution", CalculateAngleResolution(mergeFromCascade));
		CascadeMergeProgram->SetIVector("mergeToAngleResolution", CalculateAngleResolution(mergeToCascade));

		CascadeMergeProgram->SetInt("mergeFromCascade", mergeFromCascade);
		CascadeMergeProgram->SetInt("mergeToCascade", mergeToCascade);

		CascadeMergeProgram->SetVector("fromHorizontalTransform", glm::vec2(xOffset2, fromXScale));
		CascadeMergeProgram->SetVector("toHorizontalTransform", glm::vec2(xOffset1, toXScale));

		FullscreenQuad->RenderMesh();

		// DEBUG
		//break;
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

		CascadeRenderProgram->SetBool("bilinearFix", OutputBilinearFix);

		CascadeRenderProgram->SetIVector("cascade0AngleResolution", Cascade0AngularResolution);
		CascadeRenderProgram->SetIVector("cascade0ProbeResolution", Cascade0ProbeResolution);
		CascadeRenderProgram->SetIVector("cascade0Dimensions", Cascade0AngularResolution * Cascade0ProbeResolution);

		CascadeRenderProgram->SetTexture("worldTexture", FinalWorldFrameBuffer->GetTexture());
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

void RadianceCascades::DrawRectangle(Colour* colourData, glm::ivec2 position, glm::ivec2 dimensions, Colour colour)
{
	for (int y = position.y; y < std::min(position.y + dimensions.y, Height - 1); ++y)
	{
		for (int x = position.x; x < std::min(position.x + dimensions.x, Width - 1); ++x)
			colourData[y * Width + x] = colour;
	}
}

void RadianceCascades::RenderPaintBrush()
{
	RenderProgram->BindProgram();

	glm::vec2 position = Input::GetMousePosition();

	glm::mat4 model = glm::mat4();
	model = glm::translate(model, glm::vec3(position, 0.0f));
	model = glm::scale(model, glm::vec3(20.0f, 20.0f, 1.0f));

	glm::mat4 pvm = ProjectionView * model;

	double totalTime = glfwGetTime();
	glm::vec3 rgb = HsvToRgb({ std::fmodf(totalTime * 10.0f, 360.0f), 1.0f, 1.0f });

	if (Input::IsMouseDown(1))
		rgb = glm::vec3(0, 0, 0);

	RenderProgram->SetMatrix("PVM", pvm);
	RenderProgram->SetVector("colour", glm::vec4(rgb, 1.0f));

	FullscreenQuad->RenderMesh();

	RenderProgram->UnbindProgram();
}

glm::vec3 RadianceCascades::HsvToRgb(glm::vec3 hsv)
{
	double hh, p, q, t, ff;
	long i;
	glm::vec3 out;

	if (hsv.g <= 0.0) {       // < is bogus, just shuts up warnings
		out.r = hsv.z;
		out.g = hsv.z;
		out.b = hsv.z;
		return out;
	}

	hh = hsv.r;

	if (hh >= 360.0) 
		hh = 0.0;

	hh /= 60.0;
	i = (long)hh;
	ff = hh - i;
	p = hsv.z * (1.0 - hsv.y);
	q = hsv.z * (1.0 - (hsv.y * ff));
	t = hsv.z * (1.0 - (hsv.y * (1.0 - ff)));

	switch (i) 
	{
		case 0:
			out.r = hsv.z;
			out.g = t;
			out.b = p;
			break;
		case 1:
			out.r = q;
			out.g = hsv.z;
			out.b = p;
			break;
		case 2:
			out.r = p;
			out.g = hsv.z;
			out.b = t;
			break;

		case 3:
			out.r = p;
			out.g = q;
			out.b = hsv.z;
			break;
		case 4:
			out.r = t;
			out.g = p;
			out.b = hsv.z;
			break;
		case 5:
		default:
			out.r = hsv.z;
			out.g = p;
			out.b = q;
			break;
	}

	return out;
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
