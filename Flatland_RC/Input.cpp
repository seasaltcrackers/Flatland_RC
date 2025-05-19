#include "Input.h"

#include <GLFW/glfw3.h>
#include <imgui.h>


glm::vec2 Input::MousePosition;
GLFWwindow* Input::Window = nullptr;

void CursorPositionCallback(GLFWwindow* window, double xpos, double ypos)
{
	ImGuiIO& io = ImGui::GetIO();
	io.AddMousePosEvent(xpos, ypos);

	int width = 0;
	int height = 0;

	glfwGetWindowSize(window, &width, &height);
	Input::SetMousePosition(glm::vec2(xpos, height - ypos));
}

void Input::Initialise(GLFWwindow* window)
{
	Window = window;

	glfwSetCursorPosCallback(Window, CursorPositionCallback);
}

void Input::SetMousePosition(glm::vec2 mousePosition)
{
	MousePosition = mousePosition;
}

glm::vec2 Input::GetMousePosition()
{
	return MousePosition;
}

bool Input::IsMouseDown(int mouseButton)
{
	return glfwGetMouseButton(Window, mouseButton) == GLFW_PRESS;
}

