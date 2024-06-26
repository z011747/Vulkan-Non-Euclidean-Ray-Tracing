#include "Texture.h"
#include "Descriptor.h"

std::vector<std::shared_ptr<TextureData>> Texture::storedTextures = {};

std::shared_ptr<TextureData> Texture::get(std::string filePath)
{
    for (auto& tex : storedTextures) //search existing loaded textures and return if found
    {
        if (tex->path == filePath)
            return tex;
    }

	if (filePath == "")
	{

	}

    //no texture found so try loading it
    std::shared_ptr<TextureData> tex = std::make_shared<TextureData>();
    tex->loadTexture(filePath);
	tex->initSampler();
	tex->descriptor = std::make_shared<Descriptor>(tex);

	tex->textureID = (uint32_t)Texture::storedTextures.size();
	Texture::storedTextures.push_back(tex);

    return tex;
}

void Texture::cleanupTextures()
{
    for (auto& tex : storedTextures) //search existing loaded textures and return if found
    {
        tex->cleanup();
    }
}

void TextureData::loadTexture(std::string filePath)
{
	path = filePath;
	//load in image
	int texWidth, texHeight, texChannels;
	stbi_uc* pixels = stbi_load(filePath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
	VkDeviceSize imageSize = texWidth * texHeight * 4;

	if (!pixels)
	{
		loadTexture("textures/dummy.png");
		path = filePath; //make sure it doesnt duplicate
		return;
		//throw std::runtime_error("failed to load texture image!");
	}
		

	//create buffer and map memory
	AppUtil::createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);
	void* data;
	vkMapMemory(AppUtil::getDevice(), stagingBufferMemory, 0, imageSize, 0, &data);
	memcpy(data, pixels, static_cast<size_t>(imageSize));
	vkUnmapMemory(AppUtil::getDevice(), stagingBufferMemory);

	stbi_image_free(pixels); //free image now that its mapped

	AppUtil::createImage(texWidth, texHeight, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage, textureImageMemory);

	//vkBindImageMemory(AppUtil::getDevice(), textureImage, textureImageMemory, 0);

	AppUtil::transitionImageLayout(textureImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	AppUtil::copyBufferToImage(stagingBuffer, textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
	AppUtil::transitionImageLayout(textureImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	vkDestroyBuffer(AppUtil::getDevice(), stagingBuffer, nullptr);
	vkFreeMemory(AppUtil::getDevice(), stagingBufferMemory, nullptr);

	textureImageView = AppUtil::createImageView(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);

}

void TextureData::initSampler()
{
	VkSamplerCreateInfo samplerInfo{};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.anisotropyEnable = VK_TRUE;

	VkPhysicalDeviceProperties properties{};
	vkGetPhysicalDeviceProperties(AppUtil::getPhysicalDevice(), &properties);
	samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = 0.0f;

	if (vkCreateSampler(AppUtil::getDevice(), &samplerInfo, nullptr, &textureSampler) != VK_SUCCESS)
		throw std::runtime_error("failed to create texture sampler!");
}

void TextureData::cleanup()
{
	descriptor->cleanup();
	vkDestroySampler(AppUtil::getDevice(), textureSampler, nullptr);
	vkDestroyImageView(AppUtil::getDevice(), textureImageView, nullptr);
	vkDestroyImage(AppUtil::getDevice(), textureImage, nullptr);
	vkFreeMemory(AppUtil::getDevice(), textureImageMemory, nullptr);
}
