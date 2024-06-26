#pragma once

#include <vector>
#include <fstream>
#include "BaseObject.h"
#include <vulkan/vulkan.hpp>

class Shader : BaseObject
{
public: 
	VkShaderModule shaderModule;
	VkPipelineShaderStageCreateInfo shaderInfo{};

	static std::vector<char> readFile(const std::string& filename) 
	{
		std::ifstream file(filename, std::ios::ate | std::ios::binary);

		if (!file.is_open())
			throw std::runtime_error("failed to open file!");
		
		size_t fileSize = (size_t)file.tellg();
		std::vector<char> buffer(fileSize);

		file.seekg(0);
		file.read(buffer.data(), fileSize); //read file
		file.close();

		return buffer;
	}

	void load(const std::string& filename, VkShaderStageFlagBits stage)
	{
		auto code = readFile(filename);
		shaderModule = createShaderModule(code);

		shaderInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderInfo.stage = stage;
		shaderInfo.module = shaderModule;
		shaderInfo.pName = "main";


	}

	virtual void cleanup() override
	{
		vkDestroyShaderModule(AppUtil::getDevice(), shaderModule, nullptr);
	}
private: 
	VkShaderModule createShaderModule(const std::vector<char>& code);
};

