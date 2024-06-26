#pragma once
#include <vulkan/vulkan.hpp>
#include "AppUtil.h"
class RTFuncs
{
public: 
	static PFN_vkGetRayTracingShaderGroupHandlesKHR vkGetRayTracingShaderGroupHandlesKHR;
	static PFN_vkCmdTraceRaysKHR vkCmdTraceRaysKHR;
	static PFN_vkCreateRayTracingPipelinesKHR vkCreateRayTracingPipelinesKHR;
	static PFN_vkGetAccelerationStructureBuildSizesKHR vkGetAccelerationStructureBuildSizesKHR;
	static PFN_vkCreateAccelerationStructureKHR vkCreateAccelerationStructureKHR;
	static PFN_vkCmdBuildAccelerationStructuresKHR vkCmdBuildAccelerationStructuresKHR;
	static PFN_vkGetAccelerationStructureDeviceAddressKHR vkGetAccelerationStructureDeviceAddressKHR;
	static PFN_vkDestroyAccelerationStructureKHR vkDestroyAccelerationStructureKHR;

	static void load()
	{
		//load in required functions
		RTFuncs::vkGetRayTracingShaderGroupHandlesKHR = PFN_vkGetRayTracingShaderGroupHandlesKHR(vkGetDeviceProcAddr(AppUtil::getDevice(), "vkGetRayTracingShaderGroupHandlesKHR"));
		RTFuncs::vkCmdTraceRaysKHR = PFN_vkCmdTraceRaysKHR(vkGetDeviceProcAddr(AppUtil::getDevice(), "vkCmdTraceRaysKHR"));
		RTFuncs::vkCreateRayTracingPipelinesKHR = PFN_vkCreateRayTracingPipelinesKHR(vkGetDeviceProcAddr(AppUtil::getDevice(), "vkCreateRayTracingPipelinesKHR"));
		RTFuncs::vkGetAccelerationStructureBuildSizesKHR = PFN_vkGetAccelerationStructureBuildSizesKHR(vkGetDeviceProcAddr(AppUtil::getDevice(), "vkGetAccelerationStructureBuildSizesKHR"));
		RTFuncs::vkCreateAccelerationStructureKHR = PFN_vkCreateAccelerationStructureKHR(vkGetDeviceProcAddr(AppUtil::getDevice(), "vkCreateAccelerationStructureKHR"));
		RTFuncs::vkCmdBuildAccelerationStructuresKHR = PFN_vkCmdBuildAccelerationStructuresKHR(vkGetDeviceProcAddr(AppUtil::getDevice(), "vkCmdBuildAccelerationStructuresKHR"));
		RTFuncs::vkGetAccelerationStructureDeviceAddressKHR = PFN_vkGetAccelerationStructureDeviceAddressKHR(vkGetDeviceProcAddr(AppUtil::getDevice(), "vkGetAccelerationStructureDeviceAddressKHR"));
		RTFuncs::vkDestroyAccelerationStructureKHR = PFN_vkDestroyAccelerationStructureKHR(vkGetDeviceProcAddr(AppUtil::getDevice(), "vkDestroyAccelerationStructureKHR"));
	}
};

