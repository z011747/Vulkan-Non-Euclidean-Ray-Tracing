#include "RayTraceImage.h"
#include "Application.h"

void RayTraceImage::init()
{
	VkImageCreateInfo imageInfo{};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.format = VK_FORMAT_B8G8R8A8_SRGB;
	imageInfo.extent.width = AppUtil::getApp()->swapChainExtent.width;
	imageInfo.extent.height = AppUtil::getApp()->swapChainExtent.height;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	vkCreateImage(AppUtil::getDevice(), &imageInfo, nullptr, &image);

	VkMemoryRequirements memReqs;
	vkGetImageMemoryRequirements(AppUtil::getDevice(), image, &memReqs);
	VkMemoryAllocateInfo memoryAllocateInfo{};
	memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryAllocateInfo.allocationSize = memReqs.size;
	memoryAllocateInfo.memoryTypeIndex = AppUtil::findMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	vkAllocateMemory(AppUtil::getDevice(), &memoryAllocateInfo, nullptr, &memory);
	vkBindImageMemory(AppUtil::getDevice(), image, memory, 0);


	VkImageViewCreateInfo colorImageInfo{};
	colorImageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	colorImageInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	colorImageInfo.format = AppUtil::getApp()->swapChainImageFormat;
	colorImageInfo.subresourceRange = {};
	colorImageInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT | VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT;
	colorImageInfo.subresourceRange.baseMipLevel = 0;
	colorImageInfo.subresourceRange.levelCount = 1;
	colorImageInfo.subresourceRange.baseArrayLayer = 0;
	colorImageInfo.subresourceRange.layerCount = 1;
	colorImageInfo.image = image;
	vkCreateImageView(AppUtil::getDevice(), &colorImageInfo, nullptr, &view);

	
	AppUtil::transitionImageLayout(image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);
}

void RayTraceImage::cleanup()
{
	vkDestroyImageView(AppUtil::getDevice(), view, nullptr);
	vkDestroyImage(AppUtil::getDevice(), image, nullptr);
	vkFreeMemory(AppUtil::getDevice(), memory, nullptr);
}
