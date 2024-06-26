#include "ShaderBindingTable.h"
#include "Application.h"

void ShaderBindingTable::createTable(VkPhysicalDeviceRayTracingPipelinePropertiesKHR rayTracingPipelineProperties, VkPipeline pipeline)
{
	const uint32_t handleSize = rayTracingPipelineProperties.shaderGroupHandleSize;
	const uint32_t handleSizeAligned = align(rayTracingPipelineProperties.shaderGroupHandleSize, rayTracingPipelineProperties.shaderGroupHandleAlignment);
	const uint32_t sbtSize = shaders.size() * handleSizeAligned;
	std::vector<uint8_t> shaderHandleStorage(sbtSize);

	if (RTFuncs::vkGetRayTracingShaderGroupHandlesKHR(AppUtil::getDevice(), pipeline, 0, shaders.size(), sbtSize, shaderHandleStorage.data()) != VK_SUCCESS)
		throw std::runtime_error("failed");

	std::vector<RTShader> raygenShaders{};
	std::vector<RTShader> missShaders{};
	std::vector<RTShader> hitShaders{};

	for (auto& shader : shaders)
	{
		switch (shader.group)
		{
			case RAYGEN:
				raygenShaders.push_back(shader);
				break;
			case MISS:
				missShaders.push_back(shader);
				break;
			case HIT:
				hitShaders.push_back(shader);
				break;
		}
	}


	const VkBufferUsageFlags bufferUsageFlags = VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
	const VkMemoryPropertyFlags memoryUsageFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

	AppUtil::createBuffer(handleSize * raygenShaders.size(), bufferUsageFlags, memoryUsageFlags, raygen.buffer, raygen.memory);
	AppUtil::createBuffer(handleSize * missShaders.size(), bufferUsageFlags, memoryUsageFlags, miss.buffer, miss.memory);
	AppUtil::createBuffer(handleSize * hitShaders.size(), bufferUsageFlags, memoryUsageFlags, closestHit.buffer, closestHit.memory);


	raygen.deviceAddress = AppUtil::getBufferDeviceAddress(raygen.buffer);
	miss.deviceAddress = AppUtil::getBufferDeviceAddress(miss.buffer);
	closestHit.deviceAddress = AppUtil::getBufferDeviceAddress(closestHit.buffer);

	void* raygenMapped = nullptr; 
	void* missMapped = nullptr;
	void* closestHitMapped = nullptr;
	Application::checkVkResult(vkMapMemory(AppUtil::getDevice(), raygen.memory, 0, handleSize * raygenShaders.size(), 0, &raygenMapped));
	Application::checkVkResult(vkMapMemory(AppUtil::getDevice(), miss.memory, 0, handleSize * missShaders.size(), 0, &missMapped));
	Application::checkVkResult(vkMapMemory(AppUtil::getDevice(), closestHit.memory, 0, handleSize * hitShaders.size(), 0, &closestHitMapped));

	uint32_t offset = 0;
	memcpy(raygenMapped, shaderHandleStorage.data() + handleSizeAligned*offset, handleSize * raygenShaders.size());
	offset += missShaders.size(); //offset for miss shaders

	memcpy(missMapped, shaderHandleStorage.data() + handleSizeAligned*offset, handleSize * missShaders.size());
	offset += hitShaders.size(); //offset for hit shaders

	memcpy(closestHitMapped, shaderHandleStorage.data() + handleSizeAligned*offset, handleSize * hitShaders.size());
	 

	raygenAddressData.deviceAddress = raygen.deviceAddress;
	raygenAddressData.size = handleSizeAligned * raygenShaders.size();
	raygenAddressData.stride = handleSizeAligned;

	missAddressData.deviceAddress = miss.deviceAddress;
	missAddressData.size = handleSizeAligned * missShaders.size();
	missAddressData.stride = handleSizeAligned;

	hitAddressData.deviceAddress = closestHit.deviceAddress;
	hitAddressData.size = handleSizeAligned * hitShaders.size();
	hitAddressData.stride = handleSizeAligned;
}

inline uint32_t ShaderBindingTable::align(uint32_t size, uint32_t align)
{
	return (size + align - 1) & ~(align - 1);
}
