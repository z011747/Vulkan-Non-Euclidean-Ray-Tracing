#pragma once
#include "AppUtil.h"
#include "RTFuncs.h"
#include "MeshObject.h"
#include <vector>
class Application;
#include <vulkan/vulkan.hpp>

struct BLGeometry
{
	//bottom level
	VkAccelerationStructureGeometryKHR geometry;
	VkAccelerationStructureBuildRangeInfoKHR offsetData;
	VkTransformMatrixKHR transform;
	BaseObject* meshParent;
	uint32_t textureID;
};

struct TLGeometry
{
	uint64_t BLASAddress;
	VkAccelerationStructureInstanceKHR instance;
};

class AccelerationStructure
{
public: 

	static int TotalBLASSize;

	BaseObject* objectRef;

	Buffer buffer;
	VkAccelerationStructureKHR handle;
	Buffer scratchBuffer;
	Buffer instanceBuffer;
	void* instanceBufferMapped;


	void createBuffer(VkAccelerationStructureBuildSizesInfoKHR buildSizeInfo);
	VkAccelerationStructureBuildSizesInfoKHR getBuildSizeInfo(VkAccelerationStructureBuildGeometryInfoKHR accelerationStructureBuildGeometryInfo, uint32_t* triangleCount);
	void createStructure(VkAccelerationStructureTypeKHR structureType, VkAccelerationStructureBuildSizesInfoKHR buildSizeInfo);
	void buildStructure(VkAccelerationStructureBuildGeometryInfoKHR geometryInfo, std::vector<VkAccelerationStructureBuildRangeInfoKHR*> rangeInfo);

	Buffer createScratchBuffer(VkDeviceSize size);
	BLGeometry generateBLGeometry(Mesh mesh, ModelTransformUB transform);
	TLGeometry generateTLGeometry(uint64_t BLASAddress, VkTransformMatrixKHR transform, uint32_t instanceID, uint32_t shaderTableOffset);


	std::vector<BLGeometry> BLASGeometries; //needed for storing transforms to construct the tlas
	void createBLAS(std::vector<BLGeometry> geometries);
	void createTLAS(std::vector<TLGeometry> geometries);
	void updateTLAS(std::vector<TLGeometry> geometries, VkCommandBuffer commandBuffer);
	void cleanup();
};

