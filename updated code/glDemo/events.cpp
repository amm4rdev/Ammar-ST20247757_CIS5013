#include "events.h"

bool Mouse_input::isMouseActive = false;
float Mouse_input::xyCusorPos[2] = { 0, 0 };
float Mouse_input::lastXYCusorPos[2] = { 0, 0 };
int Mouse_input::pressedMouseButton = 0;

float lastX = 1000.0f / 2.0f;
float lastY = 1000.0f / 2.0f;
bool firstMouse = true;

double scrollY = 0.0f;
double _previous_scrolly_ = 0.0f;

bool Increase = 0;
bool Decrease = 0;


static void setCusorPos(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;

	Mouse_input::xyCusorPos[0] = xoffset;
	Mouse_input::xyCusorPos[1] = yoffset;
}


static void setMouseButtonCallBack(GLFWwindow* window, int button, int action, int mods)
{
	if (action == GLFW_PRESS || action == GLFW_REPEAT)
		Mouse_input::pressedMouseButton = button;
	else if(GLFW_RELEASE)
		Mouse_input::pressedMouseButton = -1;
}

static void scrollCallBack(GLFWwindow* window, double x, double y)
{
	scrollY = y;

	if (scrollY > _previous_scrolly_)
	{
		Increase = true;
		Decrease = false;
	}
	else if (scrollY < _previous_scrolly_)
	{
		Decrease = true;
		Increase = false;
	}

	_previous_scrolly_ = y;
}


Mouse_input::Mouse_input(GLFWwindow* window)
{
	glfwSetCursorPosCallback(window, (GLFWcursorposfun)setCusorPos);
	glfwSetMouseButtonCallback(window, (GLFWmousebuttonfun)setMouseButtonCallBack);
	glfwSetScrollCallback(window, scrollCallBack);
}


Mouse_Scroll_enum Mouse_input::mouse_scroll_return()
{
	Mouse_Scroll_enum f;
	f.increase = Increase;
	f.decrease = Decrease;

	Increase = false;
	Decrease = false;
	return f;
}

bool Mouse_input::mouseMoved()
{
	if (Mouse_input::lastXYCusorPos[0] != Mouse_input::xyCusorPos[0])
	{
		Mouse_input::lastXYCusorPos[0] = Mouse_input::xyCusorPos[0];
		Mouse_input::lastXYCusorPos[1] = Mouse_input::xyCusorPos[1];

		return true;
	}
	else
	{
		return false;
	}
}

bool Mouse_input::checkmouseButtons(int button)
{
	if (pressedMouseButton == button)
	{
		pressedMouseButton = -1;
		return true;
	}

	return false;
}

float const* Mouse_input::getCusorPosition()
{
	return xyCusorPos;
}
