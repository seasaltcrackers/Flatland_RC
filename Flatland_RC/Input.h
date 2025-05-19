#pragma once
#include <glm.hpp>
#include <GLFW/glfw3.h>

class Input
{
public:

	static void Initialise(GLFWwindow* window);

	static void SetMousePosition(glm::vec2 mousePosition);
	static glm::vec2 GetMousePosition();

	static bool IsMouseDown(int mouseButton);

private:

	static GLFWwindow* Window;
	static glm::vec2 MousePosition;

};

