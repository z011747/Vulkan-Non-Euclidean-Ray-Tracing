#include "Input.h"

GLFWwindow* Input::windowP = nullptr;
double Input::mouseX = 0.0;
double Input::mouseY = 0.0;
double Input::mouseMovementX = 0.0;
double Input::mouseMovementY = 0.0;

bool Input::leftClickPressed = false;
bool Input::middleClickPressed = false;
bool Input::rightClickPressed = false;

bool Input::leftClickJustPressed = false;
bool Input::middleClickJustPressed = false;
bool Input::rightClickJustPressed = false;

bool Input::leftClickJustReleased = false;
bool Input::middleClickJustReleased = false;
bool Input::rightClickJustReleased = false;

double Input::scrollY = 0.0;