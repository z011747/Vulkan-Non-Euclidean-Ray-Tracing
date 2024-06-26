#pragma once
#include <memory>
#include <vector>
#include "imgui.h"
#include "Scene.h"
#include "imgui_impl_vulkan.h"
#include "AppUtil.h"
#include "RTFuncs.h"
#include "glm/glm.hpp"
#include <glm/gtc/matrix_transform.hpp>
struct Mesh;
class Application;
#include <vulkan/vulkan.hpp>
#include "ShaderBindingTable.h"
#include "RayTraceImage.h"
#include "AccelerationStructure.h"

class Raytracer
{
public: 
	std::vector<AccelerationStructure> blases;
	//AccelerationStructure BLAS{};
	AccelerationStructure TLAS{};
	AccelerationStructure TLAS_stretched{};
	std::shared_ptr<Scene> currentScene;

	void init(std::shared_ptr<Scene> currentScene);
	void resizeImage();
	void initDescriptors();
	void initRaytracingPipeline();
	void initShaderTables();
	void initImage();
	void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
	void initBLAS();
	void initTLAS();
	void updateInstanceUB();
	void updateTLAS(VkCommandBuffer commandBuffer);
	

	void cleanup();

private:

	InstancesVIUB instanceVIUB;
	int storedInstances = 0;
	void* instanceUBMapped = nullptr;
	Buffer instanceVertexIndicesAddressBuffer;

	VkPhysicalDeviceRayTracingPipelinePropertiesKHR  rayTracingPipelineProperties{};

	VkDescriptorPool descriptorPool;
	std::vector<VkDescriptorSet> descriptorSets;
	VkDescriptorSetLayout descriptorSetLayout;

	VkPipelineLayout pipelineLayout;
	VkPipeline pipeline;

	ShaderBindingTable shaderBindingTable{};
	RayTraceImage image{};
};

