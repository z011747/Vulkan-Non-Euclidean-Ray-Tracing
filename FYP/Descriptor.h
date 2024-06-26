#pragma once

#include "AppUtil.h"
#include <array>
#include "vulkan/vulkan.hpp"
#include <vector>
#include <memory>

class Texture;
struct TextureData;

class Descriptor
{
public:
	Descriptor(std::shared_ptr<TextureData> tex);

	void cleanup()
	{
		vkDestroyDescriptorPool(AppUtil::getDevice(), descriptorPool, nullptr);
		
	}
	VkDescriptorPool descriptorPool;
	std::vector<VkDescriptorSet> descriptorSets;
};

