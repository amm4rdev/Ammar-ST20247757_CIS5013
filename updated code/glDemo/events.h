#pragma once
#include "GL/glew.h"

#include "GLFW/glfw3.h"

struct Mouse_Scroll_enum
{
	bool increase;
	bool decrease;
};

class Mouse_input
{
public:
	Mouse_input() = default;
	Mouse_input(GLFWwindow* window);
	~Mouse_input() = default;

	Mouse_Scroll_enum mouse_scroll_return();
	bool mouseMoved();
	bool checkmouseButtons(int button);
	float const* getCusorPosition();

	public:
	static bool isMouseActive;
	static int pressedMouseButton;
	static float xyCusorPos[2];
	static float lastXYCusorPos[2];
};


