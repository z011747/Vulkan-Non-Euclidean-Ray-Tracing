#pragma once
#include "glm/glm.hpp"
#include <string>
#include "AppUtil.h"
#include <array>
#include "vulkan/vulkan.hpp"
#include <vector>
#include <memory>
//#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
class Descriptor;
struct TextureData;

class Texture
{
public: 
	static std::shared_ptr<TextureData> get(std::string filePath);
	static void cleanupTextures();
//private: 
	static std::vector<std::shared_ptr<TextureData>> storedTextures;
};

struct TextureData
{
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	VkImage textureImage;
	VkDeviceMemory textureImageMemory;
	VkImageView textureImageView;
	VkSampler textureSampler;
	std::shared_ptr<Descriptor> descriptor;
	std::string path;
	uint32_t textureID;

	void loadTexture(std::string filePath);
	void initSampler();


	void cleanup();
};