#pragma once
#include "BaseObject.h"
#include "Camera.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "imgui.h"
#include "MeshObject.h"
#include <vector>
#include <string>
#include <memory>

class Scene : public BaseObject
{
private: 
	std::vector<std::shared_ptr<BaseObject>> objects;

	std::vector<std::shared_ptr<BaseObject>> portals;

	AABB hallwayBounds{};

	std::shared_ptr<Camera> camera;
	Light light;
	glm::vec4 tlasStretch = { 1.0, 5.0, 1.0, 0.0 };
public:
	Scene()
	{
		camera = std::make_shared<Camera>();
		light.color = { 1.0f, 1.0f, 1.0f, 1.0f };
light.position = { 0.2f, -6.0f, 8.0f, 0.0f };
	}

	void loadScene();

	void reload()
	{
		cleanup();
		loadScene();
	}

	int selectedObject = 0;

	virtual void update(float dt) override;
	void updatePortal(std::shared_ptr<BaseObject> obj);
	void updateManifoldPortal(std::shared_ptr<BaseObject> obj);
	virtual void draw() override
	{
		for (auto& obj : objects)
			obj->draw();
	}
	virtual void cleanup() override
	{
		for (auto& obj : objects)
			obj->cleanup();
		objects.clear();
	}

	void updateUB()
	{
		MainUniformBuffer ubo{};
		camera->applyCameraTransform(ubo);
		ubo.light = light;
		ubo.stretch = tlasStretch;
		ubo.insideHallway = hallwayBounds.pointOverlaps(camera->eye);
		AppUtil::updateUB(ubo);
	}

	void showObjectManipulator(float dt)
	{
		//create window
		ImGui::Begin("ImGUI Window");

		ImGui::Text("FPS: %f", ImGui::GetIO().Framerate);

		ImGui::Checkbox("Raytracing", &AppUtil::raytracingEnabled);


		ImGui::SeparatorText("Objects");

		//object changer
		ImGui::Text("Selected Object");
		ImGui::SameLine();
		if (ImGui::ArrowButton("##left", ImGuiDir_Left))
		{
			selectedObject--;
			if (selectedObject < 0)
				selectedObject = 0;
		}
		ImGui::SameLine();
		if (ImGui::ArrowButton("##right", ImGuiDir_Right))
		{
			selectedObject++;
			if (selectedObject > objects.size() - 1)
				selectedObject = (int)objects.size() - 1;
		}
		ImGui::SameLine();
		ImGui::Text("Idx: %d", selectedObject);

		

		auto& obj = objects[selectedObject];
		//object properties
		ImVec4 pos = ImVec4(obj->position.x, obj->position.y, obj->position.z, 0.0f);
		ImVec4 scale = ImVec4(obj->scale.x, obj->scale.y, obj->scale.z, 0.0f);
		ImVec4 angle = ImVec4(obj->angle.x, obj->angle.y, obj->angle.z, 0.0f);

		ImGui::Text("Name: %s", obj->name.c_str());

		ImGui::Text("Position");
		ImGui::DragFloat3("##pos", (float*)&pos, 0.1f);
		ImGui::Text("Scale");
		ImGui::DragFloat3("##scale", (float*)&scale, 0.1f);
		ImGui::Text("Angle");
		ImGui::DragFloat3("##angle", (float*)&angle, 0.1f);

		//update anything that might have changed
		obj->position.x = pos.x; obj->position.y = pos.y; obj->position.z = pos.z;
		obj->scale.x = scale.x; obj->scale.y = scale.y; obj->scale.z = scale.z;
		obj->angle.x = angle.x; obj->angle.y = angle.y; obj->angle.z = angle.z;


		ImGui::SeparatorText("Other");


		ImVec4 lightPos = ImVec4(light.position.x, light.position.y, light.position.z, 0.0f);
		ImGui::Text("Light Position");
		ImGui::DragFloat3("##lightPos", (float*)&lightPos, 0.1f);
		light.position.x = lightPos.x; light.position.y = lightPos.y; light.position.z = lightPos.z;
		
		ImGui::Text("Hallway Scale");
		ImGui::DragFloat3("##Hallway Scale", glm::value_ptr(tlasStretch), 0.1f);


		//ImGui::DragFloat3("Hallway Pos", glm::value_ptr(hallwayBounds.position), 0.1f);
		//ImGui::DragFloat3("Cam Pos", glm::value_ptr(camera->eye), 0.1f);
		//ImGui::Text("overlapping hallway %d", hallwayBounds.pointOverlaps(camera->eye));

		ImGui::End();
	}


	//helper funcs
	bool exists(std::string name)
	{
		for (auto& obj : objects)
			if (obj->name == name)
				return true;
		return false;
	}
	std::shared_ptr<BaseObject> get(std::string name)
	{
		for (auto& obj : objects)
			if (obj->name == name)
				return obj;
		return nullptr;
	}
	void add(std::shared_ptr<BaseObject> obj)
	{
		objects.push_back(obj);
	}
	void set(std::string name, std::shared_ptr<BaseObject> obj)
	{
		obj->name = name; //same as add but also sets the object's name
		objects.push_back(obj);
	}
	void remove(std::string name)
	{

	}
	void remove(std::shared_ptr<BaseObject> obj)
	{

	}

	auto getObjects() { return objects; }
};

