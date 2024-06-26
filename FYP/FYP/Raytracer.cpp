#include "Raytracer.h"
#include "Application.h"

const int MAX_FRAMES_IN_FLIGHT = 2;


void Raytracer::initBLAS()
{
	//load in each object
	for (auto& object : currentScene->getObjects())
	{
		if (dynamic_cast<MeshObject*>(object.get()) != nullptr) //only mesh objects (for now)
		{
			MeshObject* meshObj = (MeshObject*)object.get();
			ModelTransformUB trans;
			meshObj->applyTransform(trans); //get transform
			for (auto mesh : meshObj->getMeshes())
			{
				std::vector<BLGeometry> geometries;
				AccelerationStructure BLAS{};
				BLAS.objectRef = object.get();
				geometries.push_back(BLAS.generateBLGeometry(mesh, trans));
				BLAS.createBLAS(geometries);
				blases.push_back(BLAS); //create a seperate BLAS for each object
			}
		}
	}

	std::cout << "Total BLAS count: " << blases.size() << std::endl;
	std::cout << "Total BLAS sizes: " << AccelerationStructure::TotalBLASSize << std::endl;


}

void Raytracer::initTLAS()
{
	std::vector<TLGeometry> geometries;

	for (auto& BLAS : blases) //loop through each blas
	{	
											//if a blas has multiple geometries they can only have 1 transform unless using a transform buffer
		auto geo = TLAS.generateTLGeometry(BLAS.buffer.deviceAddress, BLAS.BLASGeometries[0].transform, BLAS.BLASGeometries[0].textureID, 0);
		geometries.push_back(geo);

		//used to store any vertex/index buffer address for the shader to use for getting uvs from a triangle
		ObjectInstanceUB viUB{}; 
		viUB.vertexAddress = BLAS.BLASGeometries[0].geometry.geometry.triangles.vertexData.deviceAddress;
		viUB.indexAddress = BLAS.BLASGeometries[0].geometry.geometry.triangles.indexData.deviceAddress;
		/*if (storedInstances == 2)
			viUB.type = 1;
		if (storedInstances == 3)
		{
			viUB.type = 2;
			auto orientation = glm::quat(glm::vec3(glm::radians(0.0f), glm::radians(0.0f), glm::radians(0.0f))); //construct quat from angles
			viUB.portalRotation = glm::mat4(orientation);
			viUB.portalTranslation = { 0.0, 4.0, 0.0, 0.0 };
		}*/
		instanceVIUB.addresses[storedInstances] = viUB;

		storedInstances++;
	}



	AppUtil::createBuffer(sizeof(InstancesVIUB), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		instanceVertexIndicesAddressBuffer.buffer, instanceVertexIndicesAddressBuffer.memory, &instanceVIUB);

	vkMapMemory(AppUtil::getDevice(), instanceVertexIndicesAddressBuffer.memory, 0, sizeof(InstancesVIUB), 0, &instanceUBMapped);




	TLAS.createTLAS(geometries);
	TLAS_stretched.createTLAS(geometries);
}

void Raytracer::updateInstanceUB()
{
	

	for (int i = 0; i < blases.size(); i++) //get object instance from blases
	{
		auto obj = blases[i].objectRef;
		instanceVIUB.addresses[i].type = obj->instanceType;

		if (obj->instanceType == 2 && obj->portalRef != nullptr)
		{
			//calculate portal offset and rotation
			auto portal = obj->portalRef;
			glm::vec3 offset = portal->position - obj->position;
			instanceVIUB.addresses[i].portalTranslation = glm::vec4(offset, 1.0);

			//glm::vec3 rotationOffset = portal->angle - obj->angle;
			auto orientation = glm::quat(glm::vec3(glm::radians(obj->angle.x), glm::radians(obj->angle.y), glm::radians(obj->angle.z)));
			auto portalOri = glm::quat(glm::vec3(glm::radians(portal->angle.x), glm::radians(portal->angle.y), glm::radians(portal->angle.z)));
			orientation = glm::inverse(orientation);
			orientation = orientation * portalOri;
			instanceVIUB.addresses[i].portalRotation = glm::mat4(orientation) * glm::scale(portal->scale / obj->scale);
		}
	}

	memcpy(instanceUBMapped, &instanceVIUB, sizeof(instanceVIUB));
}
void Raytracer::updateTLAS(VkCommandBuffer commandBuffer)
{
	//update the blas to update any transforms
	std::vector<TLGeometry> geometries;
	std::vector<TLGeometry> geometries_stretched;
	glm::vec3 stretch = glm::vec3(AppUtil::getApp()->uniformBuffer.stretch);

	for (auto& BLAS : blases)
	{
		//get new transform
		BaseObject* obj = BLAS.BLASGeometries[0].meshParent;
		ModelTransformUB trans;
		obj->applyTransform(trans);

		//trans.model = trans.model * glm::scale(glm::mat4(1), { 1.0, 1.0, 2.0 });

		auto m = glm::mat3x4(glm::transpose(trans.model));
		VkTransformMatrixKHR transMat;
		memcpy(&transMat, &m, sizeof(glm::mat3x4));

		auto geo = TLAS.generateTLGeometry(BLAS.buffer.deviceAddress, transMat, BLAS.BLASGeometries[0].textureID, 0);
		geometries.push_back(geo);

		

		trans.model = glm::scale(glm::mat4(1), stretch) * trans.model;
		auto m_s = glm::mat3x4(glm::transpose(trans.model));
		VkTransformMatrixKHR transMat_s;
		memcpy(&transMat_s, &m_s, sizeof(glm::mat3x4));

		auto geo_streched = TLAS.generateTLGeometry(BLAS.buffer.deviceAddress, transMat_s, BLAS.BLASGeometries[0].textureID, 0);
		geometries_stretched.push_back(geo_streched);
	}
	TLAS.updateTLAS(geometries, commandBuffer);
	TLAS_stretched.updateTLAS(geometries_stretched, commandBuffer);
}

void Raytracer::cleanup()
{
	image.cleanup();
	shaderBindingTable.closestHit.cleanup();
	shaderBindingTable.raygen.cleanup();
	shaderBindingTable.miss.cleanup();

	vkDestroyDescriptorSetLayout(AppUtil::getDevice(), descriptorSetLayout, nullptr);
	vkDestroyDescriptorPool(AppUtil::getDevice(), descriptorPool, nullptr);
	vkDestroyPipeline(AppUtil::getDevice(), pipeline, nullptr);
	vkDestroyPipelineLayout(AppUtil::getDevice(), pipelineLayout, nullptr);


	TLAS.cleanup();
	TLAS_stretched.cleanup();
	for (auto& BLAS : blases)
		BLAS.cleanup();
}

void Raytracer::init(std::shared_ptr<Scene> currentScene)
{
	RTFuncs::load();
	this->currentScene = currentScene;
	rayTracingPipelineProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR;
	VkPhysicalDeviceProperties2 deviceProperties2{};
	deviceProperties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
	deviceProperties2.pNext = &rayTracingPipelineProperties;
	vkGetPhysicalDeviceProperties2(AppUtil::getPhysicalDevice(), &deviceProperties2);
	
	initBLAS();
	initTLAS();
	initImage();
	initRaytracingPipeline();
	initShaderTables();
	initDescriptors();
}

void Raytracer::resizeImage()
{
	image.cleanup();
	image.init();
	//update descriptor
	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		VkDescriptorImageInfo storageImageDescriptor{};
		storageImageDescriptor.imageView = image.view;
		storageImageDescriptor.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

		VkWriteDescriptorSet resultImageWrite{};
		resultImageWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		resultImageWrite.dstSet = descriptorSets[i];
		resultImageWrite.dstBinding = 2;
		resultImageWrite.descriptorCount = 1;
		resultImageWrite.pImageInfo = &storageImageDescriptor;
		resultImageWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		vkUpdateDescriptorSets(AppUtil::getDevice(), 1, &resultImageWrite, 0, VK_NULL_HANDLE);
	}
}

void Raytracer::initDescriptors()
{
	std::vector<VkDescriptorPoolSize> poolSizes = 
	{
		{ VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, MAX_FRAMES_IN_FLIGHT},
		{ VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, MAX_FRAMES_IN_FLIGHT},
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, MAX_FRAMES_IN_FLIGHT },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, MAX_FRAMES_IN_FLIGHT },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, MAX_FRAMES_IN_FLIGHT },
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_FRAMES_IN_FLIGHT * (uint32_t)Texture::storedTextures.size() }, //texture array
	};

	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	poolInfo.pPoolSizes = poolSizes.data();
	poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

	if (vkCreateDescriptorPool(AppUtil::getDevice(), &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS)
		throw std::runtime_error("failed to create descriptor pool");
	
	std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, descriptorSetLayout);
	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = descriptorPool;
	allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
	allocInfo.pSetLayouts = layouts.data();
	descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
	if (vkAllocateDescriptorSets(AppUtil::getDevice(), &allocInfo, descriptorSets.data()) != VK_SUCCESS)
		throw std::runtime_error("failed to create descriptor pool");

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		VkWriteDescriptorSetAccelerationStructureKHR descriptorAccelerationStructureInfo{};
		descriptorAccelerationStructureInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR;
		descriptorAccelerationStructureInfo.accelerationStructureCount = 1;
		descriptorAccelerationStructureInfo.pAccelerationStructures = &TLAS.handle;

		VkWriteDescriptorSet accelerationStructureWrite{};
		accelerationStructureWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		// The specialized acceleration structure descriptor has to be chained
		accelerationStructureWrite.pNext = &descriptorAccelerationStructureInfo;
		accelerationStructureWrite.dstSet = descriptorSets[i];
		accelerationStructureWrite.dstBinding = 0;
		accelerationStructureWrite.descriptorCount = 1;
		accelerationStructureWrite.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;

		VkWriteDescriptorSet accelerationStructureWrite2{};
		accelerationStructureWrite2.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		// The specialized acceleration structure descriptor has to be chained
		VkWriteDescriptorSetAccelerationStructureKHR descriptorAccelerationStructureInfo2{};
		descriptorAccelerationStructureInfo2.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR;
		descriptorAccelerationStructureInfo2.accelerationStructureCount = 1;
		descriptorAccelerationStructureInfo2.pAccelerationStructures = &TLAS_stretched.handle;
		accelerationStructureWrite2.pNext = &descriptorAccelerationStructureInfo2;
		accelerationStructureWrite2.dstSet = descriptorSets[i];
		accelerationStructureWrite2.dstBinding = 1;
		accelerationStructureWrite2.descriptorCount = 1;
		accelerationStructureWrite2.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;

		VkDescriptorImageInfo storageImageDescriptor{};
		storageImageDescriptor.imageView = image.view;
		storageImageDescriptor.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

		VkWriteDescriptorSet resultImageWrite{};
		resultImageWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		resultImageWrite.dstSet = descriptorSets[i];
		resultImageWrite.dstBinding = 2;
		resultImageWrite.descriptorCount = 1;
		resultImageWrite.pImageInfo = &storageImageDescriptor;
		resultImageWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;

		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.buffer = AppUtil::getApp()->uniformBuffers[i];
		bufferInfo.offset = 0;
		bufferInfo.range = sizeof(MainUniformBuffer);

		VkWriteDescriptorSet uniformBufferWrite{};
		uniformBufferWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		uniformBufferWrite.dstSet = descriptorSets[i];
		uniformBufferWrite.dstBinding = 3;
		uniformBufferWrite.descriptorCount = 1;
		uniformBufferWrite.pBufferInfo = &bufferInfo;
		uniformBufferWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

		std::vector<VkDescriptorImageInfo> imageInfos;

		for (auto& tex : Texture::storedTextures)
		{
			VkDescriptorImageInfo imageInfo{};
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfo.imageView = tex->textureImageView;
			imageInfo.sampler = tex->textureSampler;
			imageInfos.push_back(imageInfo);
		}



		VkWriteDescriptorSet textureWrite{};
		textureWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		textureWrite.dstSet = descriptorSets[i];
		textureWrite.dstBinding = 5;
		textureWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		textureWrite.descriptorCount = (uint32_t)imageInfos.size();
		textureWrite.pImageInfo = imageInfos.data();
		
		//std::vector<VkDescriptorBufferInfo> viBufferInfos;

		//for (size_t j = 0; j < blases.size(); j++)
		//{
			VkDescriptorBufferInfo viBI{};
			viBI.buffer = instanceVertexIndicesAddressBuffer.buffer;
			viBI.offset = 0;
			viBI.range = sizeof(InstancesVIUB);
			//viBufferInfos.push_back(viBI);
			
		//}

		VkWriteDescriptorSet viUBWrite{};
		viUBWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		viUBWrite.dstSet = descriptorSets[i];
		viUBWrite.dstBinding = 4;
		viUBWrite.descriptorCount = 1;
		viUBWrite.pBufferInfo = &viBI;
		viUBWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;


		std::vector<VkWriteDescriptorSet> writeDescriptorSets = {
			accelerationStructureWrite,
			accelerationStructureWrite2,
			resultImageWrite,
			uniformBufferWrite,
			viUBWrite,
			textureWrite
		};
		vkUpdateDescriptorSets(AppUtil::getDevice(), static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, VK_NULL_HANDLE);
	}
}

void Raytracer::initRaytracingPipeline()
{
	
	VkDescriptorSetLayoutBinding accelerationStructureLayoutBinding{};
	accelerationStructureLayoutBinding.binding = 0;
	accelerationStructureLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
	accelerationStructureLayoutBinding.descriptorCount = 1;
	accelerationStructureLayoutBinding.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR | VK_SHADER_STAGE_MISS_BIT_KHR;

	VkDescriptorSetLayoutBinding accelerationStructureLayoutBinding2{};
	accelerationStructureLayoutBinding2.binding = 1;
	accelerationStructureLayoutBinding2.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
	accelerationStructureLayoutBinding2.descriptorCount = 1;
	accelerationStructureLayoutBinding2.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR | VK_SHADER_STAGE_MISS_BIT_KHR;

	VkDescriptorSetLayoutBinding resultImageLayoutBinding{};
	resultImageLayoutBinding.binding = 2;
	resultImageLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	resultImageLayoutBinding.descriptorCount = 1;
	resultImageLayoutBinding.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR;

	VkDescriptorSetLayoutBinding uniformBufferBinding{};
	uniformBufferBinding.binding = 3;
	uniformBufferBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uniformBufferBinding.descriptorCount = 1;
	uniformBufferBinding.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR | VK_SHADER_STAGE_MISS_BIT_KHR;

	VkDescriptorSetLayoutBinding textureBinding{};
	textureBinding.binding = 5;
	textureBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	textureBinding.descriptorCount = (uint32_t)Texture::storedTextures.size();
	textureBinding.stageFlags = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR | VK_SHADER_STAGE_ANY_HIT_BIT_KHR;

	VkDescriptorSetLayoutBinding vertBinding{};
	vertBinding.binding = 4;
	vertBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; 
	vertBinding.descriptorCount = 1;
	vertBinding.stageFlags = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR | VK_SHADER_STAGE_ANY_HIT_BIT_KHR;

	std::vector<VkDescriptorSetLayoutBinding> bindings(
	{
		accelerationStructureLayoutBinding,
		accelerationStructureLayoutBinding2,
		resultImageLayoutBinding,
		uniformBufferBinding,
		vertBinding,
		textureBinding
	});

	VkDescriptorSetLayoutCreateInfo descriptorSetlayoutCI{};
	descriptorSetlayoutCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptorSetlayoutCI.bindingCount = static_cast<uint32_t>(bindings.size());
	descriptorSetlayoutCI.pBindings = bindings.data();
	if (vkCreateDescriptorSetLayout(AppUtil::getDevice(), &descriptorSetlayoutCI, nullptr, &descriptorSetLayout) != VK_SUCCESS)
		throw std::runtime_error("failed to create");

	VkPipelineLayoutCreateInfo pipelineLayoutCI{};
	pipelineLayoutCI.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutCI.setLayoutCount = 1;
	pipelineLayoutCI.pSetLayouts = &descriptorSetLayout;
	if(vkCreatePipelineLayout(AppUtil::getDevice(), &pipelineLayoutCI, nullptr, &pipelineLayout) != VK_SUCCESS)
		throw std::runtime_error("failed to create");

	std::vector<VkPipelineShaderStageCreateInfo> shaderStages{};
	std::vector<VkRayTracingShaderGroupCreateInfoKHR> shaderGroups{};

	shaderBindingTable.init();

	for (auto& shader : shaderBindingTable.shaders)
	{
		//push create infos into vectors for the pipeline creation
		shaderStages.push_back(shader.stageInfo);
		shaderGroups.push_back(shader.groupCreateInfo);
	}
	

	VkRayTracingPipelineCreateInfoKHR rayTracingPipelineCI{};
	rayTracingPipelineCI.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR;
	rayTracingPipelineCI.stageCount = static_cast<uint32_t>(shaderStages.size());
	rayTracingPipelineCI.pStages = shaderStages.data();
	rayTracingPipelineCI.groupCount = static_cast<uint32_t>(shaderGroups.size());
	rayTracingPipelineCI.pGroups = shaderGroups.data();
	rayTracingPipelineCI.maxPipelineRayRecursionDepth = rayTracingPipelineProperties.maxRayRecursionDepth;
	rayTracingPipelineCI.layout = pipelineLayout;

	if(RTFuncs::vkCreateRayTracingPipelinesKHR(AppUtil::getDevice(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &rayTracingPipelineCI, nullptr, &pipeline) != VK_SUCCESS)
		throw std::runtime_error("failed to create");

	shaderBindingTable.cleanupShaders();
}

void Raytracer::initShaderTables()
{
	shaderBindingTable.createTable(rayTracingPipelineProperties, pipeline);
}

void Raytracer::initImage()
{
	image.init();
}


void Raytracer::recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex)
{
	//begin draw
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = 0; // Optional
	beginInfo.pInheritanceInfo = nullptr; // Optional

	if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
		throw std::runtime_error("failed to begin recording command buffer!");

	//update before drawing
	updateInstanceUB();
	updateTLAS(commandBuffer);

	//make sure memory has finished updating before rendering
	VkMemoryBarrier memoryBarrier = {};
	memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
	memoryBarrier.pNext = nullptr;
	memoryBarrier.srcAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR | VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;
	memoryBarrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR | VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;

	vkCmdPipelineBarrier(
		commandBuffer,
		VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR,
		VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR,
		0, 1, &memoryBarrier, 0, nullptr, 0, nullptr);


	//setup pipeline and then actually draw
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipeline);
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipelineLayout, 0, 1, &descriptorSets[AppUtil::getApp()->currentFrame], 0, 0);

	RTFuncs::vkCmdTraceRaysKHR(
		commandBuffer,
		&shaderBindingTable.raygenAddressData,
		&shaderBindingTable.missAddressData,
		&shaderBindingTable.hitAddressData,
		&shaderBindingTable.callableAddressData,
		AppUtil::getApp()->swapChainExtent.width,
		AppUtil::getApp()->swapChainExtent.height,
		1);

	//make swapchain image writable
	AppUtil::transitionImageLayout(commandBuffer, AppUtil::getApp()->swapChainImages[imageIndex], VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

	//setup raytracing image
	AppUtil::transitionImageLayout(commandBuffer, image.image, VK_IMAGE_LAYOUT_GENERAL,
		VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

	//copy over image
	VkImageCopy copyRegion{};
	copyRegion.srcSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
	copyRegion.srcOffset = { 0, 0, 0 };
	copyRegion.dstSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
	copyRegion.dstOffset = { 0, 0, 0 };
	copyRegion.extent = { AppUtil::getApp()->swapChainExtent.width,
		AppUtil::getApp()->swapChainExtent.height, 1 };

	vkCmdCopyImage(commandBuffer, image.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, AppUtil::getApp()->swapChainImages[imageIndex],
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);

	

	//make swapchain image presentable
	AppUtil::transitionImageLayout(commandBuffer, AppUtil::getApp()->swapChainImages[imageIndex], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

	
	//reset raytracing image
	AppUtil::transitionImageLayout(commandBuffer, image.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		VK_IMAGE_LAYOUT_GENERAL);


	//render ImGui menu on top
	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = AppUtil::getApp()->renderPass;
	renderPassInfo.framebuffer = AppUtil::getApp()->swapChainFramebuffers[imageIndex];

	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = AppUtil::getApp()->swapChainExtent;

	std::array<VkClearValue, 2> clearValues{};
	clearValues[0].color = { {0.0f, 0.0f, 0.0f, 0.0f} };
	clearValues[1].depthStencil = { 1.0f, 0 };

	renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
	renderPassInfo.pClearValues = clearValues.data();

	vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

	ImGui::Render();
	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);

	vkCmdEndRenderPass(commandBuffer);

	
	

	if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
		throw std::runtime_error("failed to record command buffer!");
}
