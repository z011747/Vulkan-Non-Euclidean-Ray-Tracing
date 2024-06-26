
#define GLFW_INCLUDE_VULKAN
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#define STB_IMAGE_IMPLEMENTATION
//#include "stb_image.h"

#include "Application.h"
#include "AppUtil.h"

int main()
{
	Application app;
	AppUtil::setApplication(&app);
	app.init();
	app.initVulkan();
	app.initScene();
	app.mainLoop();
	app.cleanup();

	return 0;
}