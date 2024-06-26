#include "AccelerationStructure.h"
#include "Application.h"

int AccelerationStructure::TotalBLASSize = 0;


VkAccelerationStructureBuildSizesInfoKHR AccelerationStructure::getBuildSizeInfo(VkAccelerationStructureBuildGeometryInfoKHR accelerationStructureBuildGeometryInfo, uint32_t* triangleCount)
{
	VkAccelerationStructureBuildSizesInfoKHR accelerationStructureBuildSizesInfo{};
	accelerationStructureBuildSizesInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;
	RTFuncs::vkGetAccelerationStructureBuildSizesKHR
	(
		AppUtil::getDevice(),
		VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
		&accelerationStructureBuildGeometryInfo,
		triangleCount,
		&accelerationStructureBuildSizesInfo
	);
	return accelerationStructureBuildSizesInfo;
}

void AccelerationStructure::createStructure(VkAccelerationStructureTypeKHR structureType, VkAccelerationStructureBuildSizesInfoKHR buildSizeInfo)
{
	VkAccelerationStructureCreateInfoKHR accelerationStructureCreateInfo{};
	accelerationStructureCreateInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
	accelerationStructureCreateInfo.buffer = buffer.buffer;
	accelerationStructureCreateInfo.size = buildSizeInfo.accelerationStructureSize;
	accelerationStructureCreateInfo.type = structureType;
	RTFuncs::vkCreateAccelerationStructureKHR(AppUtil::getDevice(), &accelerationStructureCreateInfo, nullptr, &handle);
}

void AccelerationStructure::buildStructure(VkAccelerationStructureBuildGeometryInfoKHR geometryInfo, std::vector<VkAccelerationStructureBuildRangeInfoKHR*> rangeInfo)
{
	vkResetCommandPool(AppUtil::getDevice(), AppUtil::getApp()->commandPool, 0);
	VkCommandBuffer commandBuffer = AppUtil::getApp()->commandBuffers[0];
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	RTFuncs::vkCmdBuildAccelerationStructuresKHR(
		commandBuffer,
		1,
		&geometryInfo,
		rangeInfo.data());

	VkSubmitInfo endInfo = {};
	endInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	endInfo.commandBufferCount = 1;
	endInfo.pCommandBuffers = &commandBuffer;
	vkEndCommandBuffer(commandBuffer);
	if (vkQueueSubmit(AppUtil::getApp()->graphicsQueue, 1, &endInfo, VK_NULL_HANDLE) != VK_SUCCESS)
		throw std::runtime_error("failed");
	vkDeviceWaitIdle(AppUtil::getDevice());

	//finish up
	VkAccelerationStructureDeviceAddressInfoKHR accelerationDeviceAddressInfo{};
	accelerationDeviceAddressInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR;
	accelerationDeviceAddressInfo.accelerationStructure = handle;
	buffer.deviceAddress = RTFuncs::vkGetAccelerationStructureDeviceAddressKHR(AppUtil::getDevice(), &accelerationDeviceAddressInfo);
}

Buffer AccelerationStructure::createScratchBuffer(VkDeviceSize size)
{
	//AppUtil::createBuffer(size, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, scratchBuffer.handle, scratchBuffer.memory);

	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;

	if (vkCreateBuffer(AppUtil::getDevice(), &bufferInfo, nullptr, &scratchBuffer.buffer) != VK_SUCCESS)
		throw std::runtime_error("failed to create buffer!");

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(AppUtil::getDevice(), scratchBuffer.buffer, &memRequirements);

	VkMemoryAllocateFlagsInfo memoryAllocateFlagsInfo{};
	memoryAllocateFlagsInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
	memoryAllocateFlagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.pNext = &memoryAllocateFlagsInfo;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = AppUtil::findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	if (vkAllocateMemory(AppUtil::getDevice(), &allocInfo, nullptr, &scratchBuffer.memory) != VK_SUCCESS)
		throw std::runtime_error("failed to allocate buffer memory!");
	if (vkBindBufferMemory(AppUtil::getDevice(), scratchBuffer.buffer, scratchBuffer.memory, 0) != VK_SUCCESS)
		throw std::runtime_error("failed to bind memory");

	scratchBuffer.deviceAddress = AppUtil::getBufferDeviceAddress(scratchBuffer.buffer);
	return scratchBuffer;
}

void AccelerationStructure::cleanup()
{
	RTFuncs::vkDestroyAccelerationStructureKHR(AppUtil::getDevice(), handle, nullptr);
	buffer.cleanup();
	scratchBuffer.cleanup();
	instanceBuffer.cleanup();
}

void AccelerationStructure::createBuffer(VkAccelerationStructureBuildSizesInfoKHR buildSizeInfo)
{
	VkBufferCreateInfo bufferCreateInfo{};
	bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.size = buildSizeInfo.accelerationStructureSize;
	bufferCreateInfo.usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
	if (vkCreateBuffer(AppUtil::getDevice(), &bufferCreateInfo, nullptr, &buffer.buffer) != VK_SUCCESS)
		throw std::runtime_error("failed to create buffer");
	VkMemoryRequirements memoryRequirements{};
	vkGetBufferMemoryRequirements(AppUtil::getDevice(), buffer.buffer, &memoryRequirements);
	VkMemoryAllocateFlagsInfo memoryAllocateFlagsInfo{};
	memoryAllocateFlagsInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
	memoryAllocateFlagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
	VkMemoryAllocateInfo memoryAllocateInfo{};
	memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryAllocateInfo.pNext = &memoryAllocateFlagsInfo;
	memoryAllocateInfo.allocationSize = memoryRequirements.size;
	memoryAllocateInfo.memoryTypeIndex = AppUtil::findMemoryType(memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	if (vkAllocateMemory(AppUtil::getDevice(), &memoryAllocateInfo, nullptr, &buffer.memory) != VK_SUCCESS)
		throw std::runtime_error("failed to allocate memory");
	if (vkBindBufferMemory(AppUtil::getDevice(), buffer.buffer, buffer.memory, 0) != VK_SUCCESS)
		throw std::runtime_error("failed to bind memory");
}

BLGeometry AccelerationStructure::generateBLGeometry(Mesh mesh, ModelTransformUB transform)
{
	VkDeviceAddress vertexAddress = AppUtil::getBufferDeviceAddress(mesh.vertexBuffer);
	VkDeviceAddress indexAddress = AppUtil::getBufferDeviceAddress(mesh.indexBuffer);
	//VkDeviceAddress transformAddress = AppUtil::getBufferDeviceAddress(mesh.transformBuffer);

	uint32_t maxPrimitiveCount = (uint32_t)mesh.indices.size() / 3;

	BLGeometry geo{};
	VkAccelerationStructureGeometryTrianglesDataKHR triangleData{};
	triangleData = { VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR };
	triangleData.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;  // vec3 vertex position data.
	triangleData.vertexData.deviceAddress = vertexAddress;
	triangleData.maxVertex = (uint32_t)mesh.vertices.size() - 1;
	triangleData.vertexStride = sizeof(Vertex);
	// Describe index data (32-bit unsigned int)
	triangleData.indexType = VK_INDEX_TYPE_UINT32;
	triangleData.indexData.deviceAddress = indexAddress;
	// Indicate identity transform by setting transformData to null device pointer.
	triangleData.transformData = {};
	//geo.triangleData.maxVertex = (uint32_t)mesh.vertices.size() - 1;

	geo.geometry = { VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR };
	geo.geometry.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
	geo.geometry.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;
	geo.geometry.geometry.triangles = triangleData;

	geo.offsetData.firstVertex = 0;
	geo.offsetData.primitiveCount = maxPrimitiveCount;
	geo.offsetData.primitiveOffset = 0;
	geo.offsetData.transformOffset = 0;

	geo.meshParent = mesh.parent;
	auto m = glm::mat3x4(glm::transpose(transform.model));
	memcpy(&geo.transform, &m, sizeof(VkTransformMatrixKHR));

	geo.textureID = mesh.texture->textureID;



	return geo;
}

void AccelerationStructure::createBLAS(std::vector<BLGeometry> geometries)
{
	BLASGeometries = geometries;
	std::vector<VkAccelerationStructureBuildRangeInfoKHR*> offsets;
	std::vector<uint32_t> triangleCounts;
	std::vector<VkAccelerationStructureGeometryKHR> structureGeometry;

	for (auto& geo : geometries)
	{
		structureGeometry.push_back(geo.geometry);
		triangleCounts.push_back(geo.offsetData.primitiveCount);
		offsets.push_back(&geo.offsetData);
	}

	VkAccelerationStructureBuildGeometryInfoKHR accelerationStructureBuildGeometryInfo{};
	accelerationStructureBuildGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
	accelerationStructureBuildGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
	accelerationStructureBuildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
	accelerationStructureBuildGeometryInfo.geometryCount = (uint32_t)geometries.size();
	accelerationStructureBuildGeometryInfo.pGeometries = structureGeometry.data();

	auto buildSizeInfo = getBuildSizeInfo(accelerationStructureBuildGeometryInfo, triangleCounts.data());

	std::cout << "BLAS build size: " << buildSizeInfo.accelerationStructureSize << std::endl;

	AccelerationStructure::TotalBLASSize += buildSizeInfo.accelerationStructureSize;

	createBuffer(buildSizeInfo);
	createStructure(VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR, buildSizeInfo);

	//create scratch buffer
	auto scratchBuffer = createScratchBuffer(buildSizeInfo.buildScratchSize);

	//setup build info
	VkAccelerationStructureBuildGeometryInfoKHR accelerationBuildGeometryInfo{};
	accelerationBuildGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
	accelerationBuildGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
	accelerationBuildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
	accelerationBuildGeometryInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
	accelerationBuildGeometryInfo.dstAccelerationStructure = handle;
	accelerationBuildGeometryInfo.geometryCount = geometries.size();
	accelerationBuildGeometryInfo.pGeometries = structureGeometry.data();
	accelerationBuildGeometryInfo.scratchData.deviceAddress = scratchBuffer.deviceAddress;

	buildStructure(accelerationBuildGeometryInfo, offsets);
}

TLGeometry AccelerationStructure::generateTLGeometry(uint64_t BLASAddress, VkTransformMatrixKHR transform, uint32_t instanceID, uint32_t shaderTableOffset)
{
	TLGeometry geo{};
	VkAccelerationStructureInstanceKHR rayInst{};

	rayInst.transform = transform;  // Position of the instance

	//ModelTransformUB trans;
	//meshObj->applyTransform(trans);

	//memcpy(&rayInst.transform, &trans, sizeof(rayInst.transform));

	
	rayInst.instanceCustomIndex = instanceID;                               // gl_InstanceCustomIndexEXT
	rayInst.accelerationStructureReference = BLASAddress;
	rayInst.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR;
	rayInst.mask = 0xFF;       //  Only be hit if rayMask & instance.mask != 0
	rayInst.instanceShaderBindingTableRecordOffset = shaderTableOffset;  // We will use the same hit group for all objects

	geo.BLASAddress = BLASAddress;
	geo.instance = rayInst;
	
	return geo;
}

void AccelerationStructure::createTLAS(std::vector<TLGeometry> geometries)
{
	std::vector<VkAccelerationStructureInstanceKHR> objectInstances;
	for (auto& geo : geometries)
	{
		objectInstances.push_back(geo.instance);
	}

	//create instance buffer
	instanceBuffer.cleanup();
	AppUtil::createBuffer
	(
		sizeof(VkAccelerationStructureInstanceKHR) * objectInstances.size(),
		VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		instanceBuffer.buffer,
		instanceBuffer.memory,
		objectInstances.data()
	);
	//vkMapMemory(AppUtil::getDevice(), instanceBuffer.memory, 0, sizeof(VkAccelerationStructureInstanceKHR) * objectInstances.size(), 0, &instanceBufferMapped);

	std::vector<VkAccelerationStructureGeometryKHR> instanceGeometries;

	for (int i = 0; i < geometries.size(); i++)
	{
		VkDeviceOrHostAddressConstKHR instanceDataDeviceAddress{};
		instanceDataDeviceAddress.deviceAddress = AppUtil::getBufferDeviceAddress(instanceBuffer.buffer);

		VkAccelerationStructureGeometryKHR accelerationStructureGeometry{};
		accelerationStructureGeometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
		accelerationStructureGeometry.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
		accelerationStructureGeometry.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;
		accelerationStructureGeometry.geometry.instances.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR;
		accelerationStructureGeometry.geometry.instances.arrayOfPointers = VK_FALSE;
																														//offset address
		accelerationStructureGeometry.geometry.instances.data.deviceAddress = instanceDataDeviceAddress.deviceAddress + static_cast<uint32_t>(i) * sizeof(VkAccelerationStructureInstanceKHR);

		instanceGeometries.push_back(accelerationStructureGeometry);
	}




	VkAccelerationStructureBuildGeometryInfoKHR accelerationStructureBuildGeometryInfo{};
	accelerationStructureBuildGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
	accelerationStructureBuildGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
	accelerationStructureBuildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR | VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR;
	accelerationStructureBuildGeometryInfo.geometryCount = 1;
	accelerationStructureBuildGeometryInfo.pGeometries = instanceGeometries.data();

	uint32_t primativeCount = (uint32_t)objectInstances.size();


	auto buildSizeInfo = getBuildSizeInfo(accelerationStructureBuildGeometryInfo, &primativeCount);

	std::cout << "TLAS build size: " << buildSizeInfo.accelerationStructureSize << std::endl;
	createBuffer(buildSizeInfo);
	createStructure(VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR, buildSizeInfo);

	Buffer scratchBuffer = createScratchBuffer(buildSizeInfo.buildScratchSize);

	VkAccelerationStructureBuildGeometryInfoKHR accelerationBuildGeometryInfo{};
	accelerationBuildGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
	accelerationBuildGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
	accelerationBuildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR | VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR;
	accelerationBuildGeometryInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
	accelerationBuildGeometryInfo.dstAccelerationStructure = handle;
	accelerationBuildGeometryInfo.geometryCount = 1;
	accelerationBuildGeometryInfo.pGeometries = instanceGeometries.data();
	accelerationBuildGeometryInfo.scratchData.deviceAddress = scratchBuffer.deviceAddress;

	VkAccelerationStructureBuildRangeInfoKHR accelerationStructureBuildRangeInfo{};
	accelerationStructureBuildRangeInfo.primitiveCount = (uint32_t)instanceGeometries.size();
	accelerationStructureBuildRangeInfo.primitiveOffset = 0;
	accelerationStructureBuildRangeInfo.firstVertex = 0;
	accelerationStructureBuildRangeInfo.transformOffset = 0;
	std::vector<VkAccelerationStructureBuildRangeInfoKHR*> accelerationBuildStructureRangeInfos = { &accelerationStructureBuildRangeInfo };

	buildStructure(accelerationBuildGeometryInfo, accelerationBuildStructureRangeInfos);

	
}


void AccelerationStructure::updateTLAS(std::vector<TLGeometry> geometries, VkCommandBuffer commandBuffer)
{
	std::vector<VkAccelerationStructureInstanceKHR> objectInstances;
	for (auto& geo : geometries)
	{
		objectInstances.push_back(geo.instance);
	}

	vkCmdUpdateBuffer(commandBuffer, instanceBuffer.buffer, 0, sizeof(VkAccelerationStructureInstanceKHR) * objectInstances.size(), objectInstances.data());

	//memcpy(&instanceBufferMapped, objectInstances.data(), sizeof(VkAccelerationStructureInstanceKHR) * objectInstances.size());

	std::vector<VkAccelerationStructureGeometryKHR> instanceGeometries;

	for (int i = 0; i < geometries.size(); i++)
	{
		VkDeviceOrHostAddressConstKHR instanceDataDeviceAddress{};
		instanceDataDeviceAddress.deviceAddress = AppUtil::getBufferDeviceAddress(instanceBuffer.buffer);

		VkAccelerationStructureGeometryKHR accelerationStructureGeometry{};
		accelerationStructureGeometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
		accelerationStructureGeometry.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
		accelerationStructureGeometry.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;
		accelerationStructureGeometry.geometry.instances.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR;
		accelerationStructureGeometry.geometry.instances.arrayOfPointers = VK_FALSE;
		//offset address
		accelerationStructureGeometry.geometry.instances.data.deviceAddress = instanceDataDeviceAddress.deviceAddress + static_cast<uint32_t>(i) * sizeof(VkAccelerationStructureInstanceKHR);

		instanceGeometries.push_back(accelerationStructureGeometry);
	}

	VkAccelerationStructureBuildGeometryInfoKHR accelerationBuildGeometryInfo{};
	accelerationBuildGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
	accelerationBuildGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
	accelerationBuildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR | VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR;
	accelerationBuildGeometryInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR;
	accelerationBuildGeometryInfo.srcAccelerationStructure = handle;
	accelerationBuildGeometryInfo.dstAccelerationStructure = handle;
	accelerationBuildGeometryInfo.geometryCount = 1;
	accelerationBuildGeometryInfo.pGeometries = instanceGeometries.data();
	accelerationBuildGeometryInfo.scratchData.deviceAddress = scratchBuffer.deviceAddress;

	VkAccelerationStructureBuildRangeInfoKHR accelerationStructureBuildRangeInfo{};
	accelerationStructureBuildRangeInfo.primitiveCount = (uint32_t)instanceGeometries.size();
	accelerationStructureBuildRangeInfo.primitiveOffset = 0;
	accelerationStructureBuildRangeInfo.firstVertex = 0;
	accelerationStructureBuildRangeInfo.transformOffset = 0;
	std::vector<VkAccelerationStructureBuildRangeInfoKHR*> accelerationBuildStructureRangeInfos = { &accelerationStructureBuildRangeInfo };

	//buildStructure(accelerationBuildGeometryInfo, accelerationBuildStructureRangeInfos);

	RTFuncs::vkCmdBuildAccelerationStructuresKHR(
		commandBuffer,
		1,
		&accelerationBuildGeometryInfo,
		accelerationBuildStructureRangeInfos.data());
}