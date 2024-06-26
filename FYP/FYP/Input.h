#pragma once
#include <iostream>
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
class Input
{
public:
	static GLFWwindow* windowP;
	static double mouseX;
	static double mouseY;
	static double mouseMovementX;
	static double mouseMovementY;

	static bool leftClickPressed;
	static bool middleClickPressed;
	static bool rightClickPressed;

	static bool leftClickJustPressed;
	static bool middleClickJustPressed;
	static bool rightClickJustPressed;

	static bool leftClickJustReleased;
	static bool middleClickJustReleased;
	static bool rightClickJustReleased;

	static bool keyPressed(int key)
	{
		int state = glfwGetKey(windowP, key);
		return state == GLFW_PRESS;
	}

	static void cursorMoveCallback(GLFWwindow* window, double xpos, double ypos)
	{
		//set current movement this frame
		double diffX = xpos - mouseX;
		double diffY = ypos - mouseY;
		mouseMovementX += diffX;
		mouseMovementY += diffY;

		//update position
		mouseX = xpos;
		mouseY = ypos;
	}
	static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
	{
		if (button == GLFW_MOUSE_BUTTON_LEFT)
		{
			if (action == GLFW_PRESS)
				leftClickJustPressed = true;
			else
				leftClickJustReleased = true;
			leftClickPressed = action == GLFW_PRESS;
		}
		else if (button == GLFW_MOUSE_BUTTON_RIGHT)
		{
			if (action == GLFW_PRESS)
				rightClickJustPressed = true;
			else
				rightClickJustReleased = true;
			rightClickPressed = action == GLFW_PRESS;
		}
		else if (button == GLFW_MOUSE_BUTTON_MIDDLE)
		{
			if (action == GLFW_PRESS)
				middleClickJustPressed = true;
			else
				middleClickJustReleased = true;
			middleClickPressed = action == GLFW_PRESS;
		}
	}

	static double scrollY;

	static void mouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
	{
		scrollY = yoffset;
	}
	static void update()
	{
		//reset everything after each frame
		leftClickJustPressed = false;
		leftClickJustReleased = false;
		middleClickJustPressed = false;
		middleClickJustReleased = false;
		rightClickJustPressed = false;
		rightClickJustReleased = false;

		mouseMovementX = 0.0;
		mouseMovementY = 0.0;
		scrollY = 0.0;
	}

	
};

