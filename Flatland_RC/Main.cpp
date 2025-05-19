#pragma once
#include <iostream>

#include <glew.h>
#include <GLFW/glfw3.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "RadianceCascades.h"
#include "Constants.h"
#include "Input.h"

void Update();
void Render();
void Close();

RadianceCascades* RC = nullptr;
GLFWwindow* Window = nullptr;

int main(int argc, char** argv)
{
	if (!glfwInit())
		return -1;

	Window = glfwCreateWindow(1500, 700, "Hello World", NULL, NULL);
	if (!Window)
	{
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(Window);

	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		std::cerr << "Error: " << glewGetErrorString(err) << std::endl;
		glfwTerminate();
		return -1;
	}



	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();

	// Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForOpenGL(Window, true);
	ImGui_ImplOpenGL3_Init("#version 130");

	Input::Initialise(Window);

	// Sets the clear color when calling glClear()
	glClearColor(1.0, 0.0, 0.0, 1.0); // Black

	RC = new RadianceCascades();
	RC->Initialise(Constants::TestFactor, Constants::TestFactor);


	while (!glfwWindowShouldClose(Window))
	{
		// Start the Dear ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		Update();
		Render();

		// Render ImGui frame
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(Window);
		glfwPollEvents();
	}

	Close();

	// Cleanup
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwTerminate();

	return 0;
}

void Update()
{
	RC->Update();
}

static bool show_demo_window = true;

void Render()
{
	glClear(GL_COLOR_BUFFER_BIT);
	RC->Render();
	glFinish();
}

void Close()
{
	delete RC;
	RC = nullptr;
}
