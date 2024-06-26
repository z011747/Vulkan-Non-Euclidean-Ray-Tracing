#pragma once
#include "AppUtil.h"
//class Application;
#include "RTFuncs.h"
#include <vulkan/vulkan.hpp>
#include "Shader.h"
#include <vector>

enum RTShaderType
{
	GENERAL,
	CLOSEST,
	ANY,
	INTERSECTION
};

enum RTShaderGroup
{
	RAYGEN,
	MISS,
	HIT,
	CALLABLE
};

struct RTShader
{
	Shader* shader;
	RTShaderGroup group;
	VkPipelineShaderStageCreateInfo stageInfo;
	VkRayTracingShaderGroupCreateInfoKHR groupCreateInfo;
};

class ShaderBindingTable
{
public: 

	Buffer raygen;
	Buffer miss;
	Buffer closestHit;

	void createTable(VkPhysicalDeviceRayTracingPipelinePropertiesKHR  rayTracingPipelineProperties, VkPipeline pipeline);
	uint32_t align(uint32_t size, uint32_t align);


	std::vector<RTShader> shaders{};


	VkStridedDeviceAddressRegionKHR raygenAddressData{};
	VkStridedDeviceAddressRegionKHR missAddressData{};
	VkStridedDeviceAddressRegionKHR hitAddressData{};
	VkStridedDeviceAddressRegionKHR callableAddressData{};

	VkRayTracingShaderGroupCreateInfoKHR generateShaderGroupCreateInfo(VkRayTracingShaderGroupTypeKHR type, RTShaderType shaderType)
	{
		VkRayTracingShaderGroupCreateInfoKHR shaderGroup{};
		shaderGroup.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
		shaderGroup.type = type;
		shaderGroup.generalShader = VK_SHADER_UNUSED_KHR;
		shaderGroup.closestHitShader = VK_SHADER_UNUSED_KHR;
		shaderGroup.anyHitShader = VK_SHADER_UNUSED_KHR;
		shaderGroup.intersectionShader = VK_SHADER_UNUSED_KHR;

		switch (shaderType) //set stage index for type
		{
			case GENERAL:
				shaderGroup.generalShader = static_cast<uint32_t>(shaders.size());
				break;
			case CLOSEST:
				shaderGroup.closestHitShader = static_cast<uint32_t>(shaders.size());
				break;
			case ANY:
				shaderGroup.anyHitShader = static_cast<uint32_t>(shaders.size());
				break;
			case INTERSECTION:
				shaderGroup.intersectionShader = static_cast<uint32_t>(shaders.size());
				break;
		}

		return shaderGroup;
	}


	void generateShader(const char* file, VkShaderStageFlagBits stage, VkRayTracingShaderGroupTypeKHR type, RTShaderType shaderType, RTShaderGroup shaderGroup)
	{
		RTShader shader{};
		shader.shader = new Shader();
		shader.shader->load(file, stage);
		shader.stageInfo = shader.shader->shaderInfo;
		shader.groupCreateInfo = generateShaderGroupCreateInfo(type, shaderType);
		shader.group = shaderGroup;
		shaders.push_back(shader);
	}



	void init()
	{
		
		generateShader("shaders/raygen.spv", VK_SHADER_STAGE_RAYGEN_BIT_KHR, VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR, GENERAL, RAYGEN);
		generateShader("shaders/miss.spv", VK_SHADER_STAGE_MISS_BIT_KHR, VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR, GENERAL, MISS);
		generateShader("shaders/shadow.spv", VK_SHADER_STAGE_MISS_BIT_KHR, VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR, GENERAL, MISS);
		generateShader("shaders/closesthit.spv", VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR, VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR, CLOSEST, HIT);


	}

	void cleanupShaders()
	{
		for (auto& shader : shaders)
		{
			shader.shader->cleanup();
			delete shader.shader;
		}
	}
};

