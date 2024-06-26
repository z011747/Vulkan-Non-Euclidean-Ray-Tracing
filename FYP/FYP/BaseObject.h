#pragma once
#include <string>
#include "Structures.h"
#include <vulkan/vulkan.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
class BaseObject
{
public:
	glm::vec3 position = { 0.0f,0.0f,0.0f };
	glm::vec3 scale = { 1.0f,1.0f,1.0f };
	glm::vec3 angle = { 90.0f,0.0f,0.0f };
	glm::quat orientation = { 0.0f,0.0f,0.0f,1.0f};

	BaseObject() { };
	~BaseObject() { cleanup(); }
	virtual void update(float dt) {}
	virtual void draw() {}
	virtual void applyTransform(ModelTransformUB& transform) 
	{
		orientation = glm::quat(glm::vec3(glm::radians(angle.x), glm::radians(angle.y), glm::radians(angle.z))); //construct quat from angles
		transform.model = (glm::translate(glm::mat4(1.0f), position)) * (glm::mat4(orientation)) * (glm::scale(glm::mat4(1.0f), scale));
		if (parent != nullptr)
		{
			auto temp = transform.model;
			parent->applyTransform(transform);
			transform.model = transform.model * temp;
		}
	}
	virtual void cleanup() {}
	std::string name = "";
	std::string type = "";

	int instanceType = 0;
	BaseObject* portalRef = nullptr;
	BaseObject* parent = nullptr;
	OBB obb{};
	bool wasCollidingWithPlayer = false;
	bool inFrontOfPortal = false;
};

