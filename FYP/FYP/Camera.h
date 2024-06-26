#pragma once
#include <iostream>
#include "BaseObject.h"
#include "AppUtil.h"
#include "Input.h"
#include <math.h>
#include <cmath>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>


class Camera :
    public BaseObject
{
public: 

    Camera()
    {
        BaseObject::BaseObject();
        updateEyePosition();
    }

    float fov = 90.0f;
    float projNear = 0.01f;
    float projFar = 100.0f;
    glm::vec3 eye = { -2.0f, -4.0f, 1.5f };
    glm::vec3 center = { 0.0f, 0.0f, 0.0f };
    glm::vec3 up = { 0.0f, 0.0f, 1.0f };
    glm::vec3 scale = { 1.0f, 1.0f, 1.0f };
    glm::vec3 worldScale = { 1.0f,1.0f,1.0f };

    void applyCameraTransform(MainUniformBuffer& transform)
    {
        transform.view = glm::scale(worldScale) * glm::scale(scale) * glm::lookAt(eye, center+eye, up);
        
        transform.proj = glm::perspective(glm::radians(fov), AppUtil::getAspectRatio(), projNear, projFar);
        transform.proj[1][1] *= -1; //flip to match properly with vulkan
    }


    float distanceFromCenter = 2.0f;
    glm::vec3 cameraAngle = { glm::radians(90.0f), 0.0f, 0.0f};

    glm::vec3 cameraSpeed = { 1.0f, 1.0f, 1.0f };

    virtual void update(float dt) override
    {
        //camera controls

        if (Input::keyPressed('D'))
            cameraAngle.x -= dt * 2.0f * scale.x * scale.z;
        if (Input::keyPressed('A'))
            cameraAngle.x += dt * 2.0f * scale.x * scale.z;

        updateEyePosition();

        auto inverseScale = 1.0f / worldScale; //make speed affected by world scale

        if (Input::keyPressed('W'))
            eye += inverseScale * center * dt * 3.0f * cameraSpeed * scale.z;
        if (Input::keyPressed('S'))
            eye -= inverseScale * center * dt * 3.0f * cameraSpeed * scale.z;


        if (Input::keyPressed('Z'))
            cameraAngle.z -= dt * 2.0f;
        if (Input::keyPressed('X'))
            cameraAngle.z += dt * 2.0f;
        //eye.z = 1.5f * cameraSpeed.z ;

        cameraAngle.z = std::lerp(cameraAngle.z, 0.0f, dt * 5.0f);
        cameraAngle.y = std::lerp(cameraAngle.y, 0.0f, dt * 5.0f);
        eye.z = std::lerp(eye.z, 1.5f * inverseScale.z, dt * 5.0f);
    }

    void updateEyePosition()
    {
        float theta = cameraAngle.x;
        float phi = cameraAngle.y;

        //center.x = sin(phi) * cos(theta);
        //center.z = cos(phi);
        //center.y = sin(phi) * sin(theta);
        //glm::normalize(center);

        up.x = sin(cameraAngle.z);
        up.z = cos(cameraAngle.z);

        center.x = cos(theta);
        center.z = sin(phi);
        center.y = sin(theta);
        glm::normalize(center);

        //eye *= distanceFromCenter;
    }
};

