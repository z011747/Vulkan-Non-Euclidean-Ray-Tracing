#pragma once
class Application;
#include "BaseObject.h"
#include "Structures.h"
#include "AppUtil.h"
#include <memory>
#include "Texture.h"
#include <vector>

struct Mesh;

class MeshObject :
	public BaseObject
{
private:
	std::vector<Mesh> meshes = {};
public: 
	MeshObject() { };

	void loadOBJ(std::string path);

	void loadQuad(std::string texturePath, float uvScale = 1.0f, std::vector<glm::vec3> colors = { { 1.0f, 0.0f, 0.0f } , {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f, 1.0f} });
	
	virtual void update(float dt) override;
	virtual void draw() override;
	virtual void cleanup() override;

	auto getMeshes() { return meshes; }
};

struct Mesh
{
	std::shared_ptr<TextureData> texture;
	std::vector<Vertex> vertices = {};
	std::vector<uint32_t> indices = {};

	VkBuffer vertexBuffer;
	VkDeviceMemory vertexBufferMemory;
	VkBuffer indexBuffer;
	VkDeviceMemory indexBufferMemory;
	BaseObject* parent;
	//VkBuffer transformBuffer;
	//VkDeviceMemory transformBufferMemory;
	//void* mappedTransformBuffer;

	void makeQuad(std::vector<glm::vec3> colors, float uvScale = 1.0f)
	{
		vertices.push_back({ {-0.5f, -0.5f, 0.0f}, colors[0], {0.0f, 1.0f * uvScale}, {0.0f, 0.0f, 1.0f}});
		vertices.push_back({ {0.5f, -0.5f, 0.0f}, colors[1], {1.0f * uvScale, 1.0f * uvScale},{0.0f, 0.0f, 1.0f} });
		vertices.push_back({ {0.5f, 0.5f, 0.0f}, colors[2], {1.0f * uvScale, 0.0f}, {0.0f, 0.0f, 1.0f} });
		vertices.push_back({ {-0.5f, 0.5f, 0.0f}, colors[3], {0.0f, 0.0f}, {0.0f, 0.0f, 1.0f} });
		indices.push_back(0);
		indices.push_back(1);
		indices.push_back(2);
		indices.push_back(2);
		indices.push_back(3);
		indices.push_back(0);
	}

	void initVertexBuffer()
	{
		//for (auto& vert : vertices)
			//vert.textureID = texture->textureID; //set texture id on each verts
		VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		AppUtil::createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

		void* data;
		vkMapMemory(AppUtil::getDevice(), stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, vertices.data(), (size_t)bufferSize);
		vkUnmapMemory(AppUtil::getDevice(), stagingBufferMemory);

		AppUtil::createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer, vertexBufferMemory);
		AppUtil::copyBuffer(stagingBuffer, vertexBuffer, bufferSize);
		//destroy staging buffer
		vkDestroyBuffer(AppUtil::getDevice(), stagingBuffer, nullptr);
		vkFreeMemory(AppUtil::getDevice(), stagingBufferMemory, nullptr);
	}
	void initIndexBuffer()
	{
		VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		AppUtil::createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

		void* data;
		vkMapMemory(AppUtil::getDevice(), stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, indices.data(), (size_t)bufferSize);
		vkUnmapMemory(AppUtil::getDevice(), stagingBufferMemory);

		AppUtil::createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer, indexBufferMemory);

		AppUtil::copyBuffer(stagingBuffer, indexBuffer, bufferSize);

		vkDestroyBuffer(AppUtil::getDevice(), stagingBuffer, nullptr);
		vkFreeMemory(AppUtil::getDevice(), stagingBufferMemory, nullptr);
	}

	//void initTransformBuffer()
	//{
	//	VkDeviceSize bufferSize = sizeof(VkTransformMatrixKHR);

	//	VkBuffer stagingBuffer;
	//	VkDeviceMemory stagingBufferMemory;
	//	AppUtil::createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

	//	VkTransformMatrixKHR transform = 
	//	{
	//		1.0f, 0.0f, 0.0f, 0.0f,
	//		0.0f, 1.0f, 0.0f, 0.0f,
	//		0.0f, 0.0f, 1.0f, 0.0f 
	//	};

	//	void* data;
	//	vkMapMemory(AppUtil::getDevice(), stagingBufferMemory, 0, bufferSize, 0, &data);
	//	memcpy(data, &transform, (size_t)bufferSize);
	//	vkUnmapMemory(AppUtil::getDevice(), stagingBufferMemory);

	//	AppUtil::createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, transformBuffer, transformBufferMemory);

	//	AppUtil::copyBuffer(stagingBuffer, transformBuffer, bufferSize);

	//	//vkMapMemory(AppUtil::getDevice(), transformBufferMemory, 0, sizeof(VkTransformMatrixKHR), 0, &mappedTransformBuffer);

	//	vkDestroyBuffer(AppUtil::getDevice(), stagingBuffer, nullptr);
	//	vkFreeMemory(AppUtil::getDevice(), stagingBufferMemory, nullptr);
	//}

	//void updateTransformBuffer(ModelTransformUB& trans)
	//{
	//	//void* mappedTransformBuffer;
	//	//vkMapMemory(AppUtil::getDevice(), transformBufferMemory, 0, sizeof(VkTransformMatrixKHR), 0, &mappedTransformBuffer);
	//	//memcpy(mappedTransformBuffer, &trans, sizeof(VkTransformMatrixKHR));
	//	//vkUnmapMemory(AppUtil::getDevice(), transformBufferMemory);
	//}

	void cleanup()
	{
		vkDestroyBuffer(AppUtil::getDevice(), indexBuffer, nullptr);
		vkFreeMemory(AppUtil::getDevice(), indexBufferMemory, nullptr);
		vkDestroyBuffer(AppUtil::getDevice(), vertexBuffer, nullptr);
		vkFreeMemory(AppUtil::getDevice(), vertexBufferMemory, nullptr);		
		//vkDestroyBuffer(AppUtil::getDevice(), transformBuffer, nullptr);
		//vkFreeMemory(AppUtil::getDevice(), transformBufferMemory, nullptr);
	}

};


