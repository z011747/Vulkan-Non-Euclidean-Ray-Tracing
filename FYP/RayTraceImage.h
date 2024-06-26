#pragma once
#include "AppUtil.h"
class Application;
#include <vulkan/vulkan.hpp>
class RayTraceImage
{
public: 

	VkDeviceMemory memory;
	VkImage image;
	VkImageView view;
	VkFormat format;

	void init();

	void cleanup();
};

