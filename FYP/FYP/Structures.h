#pragma once

#include "glm/glm.hpp"
#include <string>
#include "AppUtil.h"
#include <array>
#include "vulkan/vulkan.hpp"
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
//#define STB_IMAGE_IMPLEMENTATION
//#include "stb_image.h"

struct Vertex
{
	glm::vec3 pos;
	glm::vec3 color;
	glm::vec2 texCoord;
	glm::vec3 normal;

	static VkVertexInputBindingDescription getBindingDescription() 
	{
		VkVertexInputBindingDescription bindingDescription{};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(Vertex);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return bindingDescription;
	}
	static std::array<VkVertexInputAttributeDescription, 4> getAttributeDescriptions() 
	{
		std::array<VkVertexInputAttributeDescription, 4> attributeDescriptions{};

		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Vertex, pos);

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex, color);

		attributeDescriptions[2].binding = 0;
		attributeDescriptions[2].location = 2;
		attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

		attributeDescriptions[3].binding = 0;
		attributeDescriptions[3].location = 3;
		attributeDescriptions[3].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[3].offset = offsetof(Vertex, normal);

		//attributeDescriptions[3].binding = 0;
		//attributeDescriptions[3].location = 3;
		//attributeDescriptions[3].format = VK_FORMAT_R32_UINT;
		//attributeDescriptions[3].offset = offsetof(Vertex, textureID);

		return attributeDescriptions;
	}
};

struct ObjectInstanceUB
{
	glm::mat4 portalRotation;
	alignas(8) uint64_t vertexAddress;
	alignas(8) uint64_t indexAddress;
	alignas(16) glm::vec4 portalTranslation;
	alignas(16) unsigned int type = 0;
};

struct InstancesVIUB
{
	ObjectInstanceUB addresses[128];
};

struct Light
{
	glm::vec4 position;
	glm::vec4 color;
};


struct MainUniformBuffer
{
	glm::mat4 view;
	glm::mat4 proj;
	Light light; 
	glm::vec4 stretch;
	bool insideHallway;
};

struct ModelTransformUB
{
	glm::mat4 model;
};


struct AABB
{
	glm::vec3 position{};
	glm::vec3 size{};
	bool pointOverlaps(glm::vec3& point)
	{

		return (std::abs(position.x - point.x) * 2 < (size.x)) &&
			(std::abs(position.y - point.y) * 2 < (size.y)) &&
			(std::abs(position.z - point.z) * 2 < (size.z));

		return false;
	}
};

struct OBB
{
	glm::vec3 position{};
	glm::vec3 size = {2.0, 1.0, 2.0};

	
	//simple point detection
	bool pointOverlaps(glm::vec3 point, glm::vec3& angle)
	{
		auto orientation = glm::quat(glm::vec3(glm::radians(angle.x), glm::radians(angle.y), glm::radians(angle.z)));
		auto mat = glm::translate(glm::mat4(1.0), position) * glm::mat4(orientation);
		glm::vec4 translatedPos = glm::inverse(mat) * glm::vec4(point, 1.0);

		return (std::abs(translatedPos.x) * 2 < (size.x)) &&
			(std::abs(translatedPos.y) * 2 < (size.y)) &&
			(std::abs(translatedPos.z) * 2 < (size.z));
		return false;
	}

	//front/back detection for portals
	bool isInFrontY(glm::vec3 point, glm::vec3& angle)
	{
		auto orientation = glm::quat(glm::vec3(glm::radians(angle.x), glm::radians(angle.y), glm::radians(angle.z)));
		auto mat = glm::translate(glm::mat4(1.0), position) * glm::mat4(orientation);
		glm::vec4 translatedPos = glm::inverse(mat) * glm::vec4(point, 1.0);

		return translatedPos.y > 0.0;
	}
};