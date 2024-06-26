#include "MeshObject.h"
#define TINYOBJLOADER_IMPLEMENTATION
#include "libraries/tiny_obj_loader.h"
#include "Descriptor.h"
#include "Application.h"

void MeshObject::loadOBJ(std::string path)
{
	
	tinyobj::ObjReaderConfig reader_config;
	reader_config.mtl_search_path = ""; // Path to material files
	

	tinyobj::ObjReader reader;
	

	if (!reader.ParseFromFile(path, reader_config))
		if (!reader.Error().empty())
			throw std::runtime_error(reader.Error());

	auto& attrib = reader.GetAttrib();
	auto& shapes = reader.GetShapes();
	auto& materials = reader.GetMaterials();

	for (const auto& shape : shapes)
	{
		Mesh mesh;

		auto& material = materials[shape.mesh.material_ids[0]];
		mesh.texture = Texture::get(material.diffuse_texname);

		for (const auto& index : shape.mesh.indices)
		{
			Vertex vertex{};

			vertex.pos =
			{
				attrib.vertices[3 * index.vertex_index + 0],
				attrib.vertices[3 * index.vertex_index + 1],
				attrib.vertices[3 * index.vertex_index + 2]
			};

			vertex.normal =
			{
				attrib.normals[3 * index.normal_index + 0],
				attrib.normals[3 * index.normal_index + 1],
				attrib.normals[3 * index.normal_index + 2]
			};

			vertex.texCoord =
			{
				attrib.texcoords[2 * index.texcoord_index + 0],
				1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
			};
			
			vertex.color = { material.ambient[0], material.ambient[1], material.ambient[2] };

			mesh.vertices.push_back(vertex);
			mesh.indices.push_back(mesh.indices.size());
		}

		
		mesh.initVertexBuffer();
		mesh.initIndexBuffer();
		mesh.parent = this;
		/*mesh.initTransformBuffer();*/
		meshes.push_back(mesh);
	}
	
}

void MeshObject::loadQuad(std::string texturePath, float uvScale, std::vector<glm::vec3> colors)
{
	Mesh mesh;
	mesh.texture = Texture::get(texturePath);
	mesh.makeQuad(colors, uvScale);
	mesh.initVertexBuffer();
	mesh.initIndexBuffer();
	mesh.parent = this;
	/*mesh.initTransformBuffer();*/
	
	meshes.push_back(mesh);
}

void MeshObject::update(float dt)
{

}

void MeshObject::draw()
{
	//update model transform
	ModelTransformUB transform{};
	applyTransform(transform);
	//loop through each mesh
	for (auto& mesh : meshes)
	{
		if (AppUtil::raytracingEnabled)
		{
			//VkCommandBuffer cb = AppUtil::getApp()->commandBuffers[AppUtil::getApp()->currentFrame];
			//vkCmdUpdateBuffer(cb, mesh.transformBuffer, 0, sizeof(VkTransformMatrixKHR), &transform.model);
		}
		else
		{
			//AppUtil::getApp()->drawObject(vertexBuffer, indexBuffer, indices.size(), transform);

			VkBuffer vertexBuffers[] = { mesh.vertexBuffer };
			VkDeviceSize offsets[] = { 0 };
			VkCommandBuffer cb = AppUtil::getApp()->commandBuffers[AppUtil::getApp()->currentFrame];
			vkCmdBindVertexBuffers(cb, 0, 1, vertexBuffers, offsets); //bind buffer and draw
			vkCmdBindIndexBuffer(cb, mesh.indexBuffer, 0, VK_INDEX_TYPE_UINT32);
			vkCmdBindDescriptorSets(cb, VK_PIPELINE_BIND_POINT_GRAPHICS, AppUtil::getApp()->pipelineLayout, 0, 1, &(mesh.texture->descriptor->descriptorSets[AppUtil::getApp()->currentFrame]), 0, nullptr);
			vkCmdPushConstants(cb, AppUtil::getApp()->pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(ModelTransformUB), &transform);
			vkCmdDrawIndexed(cb, static_cast<uint32_t>(mesh.indices.size()), 1, 0, 0, 0);
		}


	}
}

void MeshObject::cleanup()
{
	for (auto& mesh : meshes)
		mesh.cleanup();
}
