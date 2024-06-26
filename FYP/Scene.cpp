#include "Scene.h"
void Scene::loadScene()
{
    //load objects for scene


    auto obj2 = std::make_shared<MeshObject>();
    obj2->loadQuad("textures/olive.jpg");
    obj2->position.z = 0.5f;
    obj2->position.y = 0.5f;
    add(obj2);

    
    {
        auto obj = std::make_shared<MeshObject>();
        obj->loadOBJ("models/sphere/sphere.obj");
        obj->position = { 2.0, -3.0, 1.0 };
        add(obj);
        obj->name = "Sphere";
        obj->instanceType = 1; //reflective
    }

    //helper functions
    auto makePortalFrame = [&](std::shared_ptr<MeshObject> p)
    {
        auto portalTop = std::make_shared<MeshObject>();
        portalTop->loadOBJ("models/quad/quad.obj");
        portalTop->parent = p.get();
        portalTop->position.z = 1.0f;
        portalTop->scale.z = 0.1f;
        add(portalTop);

        auto portalLeft = std::make_shared<MeshObject>();
        portalLeft->loadOBJ("models/quad/quad.obj");
        portalLeft->parent = p.get();
        portalLeft->position.x = 1.0f;
        portalLeft->angle.z = 90;
        portalLeft->angle.x = 0;
        portalLeft->scale.x = 0.1f;
        add(portalLeft);

        auto portalRight = std::make_shared<MeshObject>();
        portalRight->loadOBJ("models/quad/quad.obj");
        portalRight->parent = p.get();
        portalRight->position.x = -1.0f;
        portalRight->angle.z = 90;
        portalRight->angle.x = 0;
        portalRight->scale.x = 0.1f;
        add(portalRight);
    };

    auto makeTextLabel = [&](std::shared_ptr<MeshObject> obj, std::string path)
    {
        auto label = std::make_shared<MeshObject>();
        //label->loadOBJ("models/quad/quad.obj");
        label->loadQuad(path, 1.0, { {1.0, 1.0, 1.0},{1.0, 1.0, 1.0},{1.0, 1.0, 1.0},{1.0, 1.0, 1.0} });
        label->position = obj->position;
        label->position.z += 2.0f;
        label->scale.x = 2.0f;
        add(label);
        return label;
    };


    auto controlsLabel = makeTextLabel(obj2, "textures/fyp_controls.png");
    controlsLabel->position.z += 2.0f;
    controlsLabel->scale *= 2.0f;

    //create basic portals
    auto portal = std::make_shared<MeshObject>();
    portal->loadOBJ("models/quad/quad.obj");
    portal->angle.x = 0.0f;
    portal->position.z = 1.0f;
    portal->position.x = 2.5f;
    portal->position.y = 3.5f;
    portal->instanceType = 2; //set type to be an actual portal
    portal->name = "Portal 1";
    portals.push_back(portal);
    add(portal);

    auto portal2 = std::make_shared<MeshObject>();
    portal2->loadOBJ("models/quad/quad.obj");
    portal2->angle.x = 0.0f;
    portal2->position.z = 1.0f;
    portal2->position.x = 2.5f;
    portal2->position.y = 7.5f;
    portal2->name = "Portal 2";
    portals.push_back(portal2);
    add(portal2);

    portal->instanceType = 2;
    portal2->instanceType = 2;
    portal->portalRef = portal2.get();
    portal2->portalRef = portal.get();
    makePortalFrame(portal);
    makePortalFrame(portal2);

    auto portalLabel = makeTextLabel(portal, "textures/fyp_portals.png");

    //mirror cube
    {
        auto mirrorCube = std::make_shared<MeshObject>();
        mirrorCube->loadOBJ("models/cubeInv/cube.obj");
        mirrorCube->position = { -50.0f, -4.0, 2.1 };
        mirrorCube->scale *= 3.0;
        mirrorCube->instanceType = 1;
        mirrorCube->name = "Mirror Cube";
        portals.push_back(mirrorCube);
        add(mirrorCube);

        auto monkey = std::make_shared<MeshObject>();
        monkey->loadOBJ("models/monkey/monkey.obj");
        monkey->position = mirrorCube->position;
        monkey->position.z += 1.0;
        //monkey->parent = mirrorCube.get();
        monkey->scale *= 0.5f;
        monkey->name = "Monkey1";
        add(monkey);

        //portals to get in and out
        auto manifoldPortalEnt = std::make_shared<MeshObject>();
        manifoldPortalEnt->loadOBJ("models/quad/quad.obj");
        manifoldPortalEnt->angle.x = 0.0f;
        manifoldPortalEnt->position.z = 1.0f;
        manifoldPortalEnt->position.x = -5.5f;
        manifoldPortalEnt->position.y = 1.5f;
        manifoldPortalEnt->instanceType = 2;
        manifoldPortalEnt->name = "MCube Portal Ent";
        portals.push_back(manifoldPortalEnt);
        add(manifoldPortalEnt);

        auto manifoldPortalExit = std::make_shared<MeshObject>();
        manifoldPortalExit->loadOBJ("models/quad/quad.obj");
        manifoldPortalExit->angle.x = 0.0f;
        manifoldPortalExit->position = mirrorCube->position;
        manifoldPortalExit->position.z = 1.0f;
        manifoldPortalExit->position.x += 0.0f;
        manifoldPortalExit->position.y += 0.0f;
        manifoldPortalExit->name = "MCube Portal Exit";
        portals.push_back(manifoldPortalExit);
        add(manifoldPortalExit);

        manifoldPortalEnt->instanceType = 2;
        manifoldPortalExit->instanceType = 2;
        manifoldPortalEnt->portalRef = manifoldPortalExit.get();
        manifoldPortalExit->portalRef = manifoldPortalEnt.get();
        makePortalFrame(manifoldPortalEnt);
        makePortalFrame(manifoldPortalExit);

        auto portalLabel = makeTextLabel(manifoldPortalEnt, "textures/fyp_cube.png");
    }

    //torus
    {

        auto torus = std::make_shared<MeshObject>();
        torus->loadOBJ("models/cubeInv/cube.obj");
        torus->position = { -70.0f, -1.0, 2.1 };
        torus->scale *= 3.0;
        torus->instanceType = 3;
        torus->name = "Torus Cube";
        portals.push_back(torus);
        add(torus);

        auto monkey = std::make_shared<MeshObject>();
        monkey->loadOBJ("models/monkey/monkey.obj");
        monkey->position = torus->position;
        monkey->position.z += 1.0;
        //monkey->parent = mirrorCube.get();
        monkey->scale *= 0.5f;
        monkey->name = "Monkey2";
        add(monkey);


        auto manifoldPortalEnt = std::make_shared<MeshObject>();
        manifoldPortalEnt->loadOBJ("models/quad/quad.obj");
        manifoldPortalEnt->angle.x = 0.0f;
        manifoldPortalEnt->position.z = 1.0f;
        manifoldPortalEnt->position.x = -8.5f;
        manifoldPortalEnt->position.y = 1.5f;
        manifoldPortalEnt->instanceType = 2;
        manifoldPortalEnt->name = "Torus Portal Ent";
        portals.push_back(manifoldPortalEnt);
        add(manifoldPortalEnt);

        auto manifoldPortalExit = std::make_shared<MeshObject>();
        manifoldPortalExit->loadOBJ("models/quad/quad.obj");
        manifoldPortalExit->angle.x = 0.0f;
        manifoldPortalExit->position = torus->position;
        manifoldPortalExit->position.z = 1.0f;
        manifoldPortalExit->position.x += 0.0f;
        manifoldPortalExit->position.y += 0.0f;
        manifoldPortalExit->name = "Torus Portal Exit";
        portals.push_back(manifoldPortalExit);
        add(manifoldPortalExit);

        manifoldPortalEnt->instanceType = 2;
        manifoldPortalExit->instanceType = 2;
        manifoldPortalEnt->portalRef = manifoldPortalExit.get();
        manifoldPortalExit->portalRef = manifoldPortalEnt.get();
        makePortalFrame(manifoldPortalEnt);
        makePortalFrame(manifoldPortalExit);

        auto portalLabel = makeTextLabel(manifoldPortalEnt, "textures/fyp_torus.png");
        auto manifoldLabel = makeTextLabel(manifoldPortalEnt, "textures/fyp_manifolds.png");
        manifoldLabel->scale *= 2.0;
        manifoldLabel->position.z += 2.0f;
    }

    //mirrored doed
    {

        auto dode = std::make_shared<MeshObject>();
        dode->loadOBJ("models/dodecahedron/dodeInv.obj");
        dode->position = { -100.0f, 3.0, 0.1 };
        dode->instanceType = 1;
        dode->scale *= 3.0;
        dode->name = "Dodecahedron";
        //portals.push_back(dode);
        add(dode);

        auto monkey = std::make_shared<MeshObject>();
        monkey->loadOBJ("models/monkey/monkey.obj");
        monkey->position = dode->position;
        monkey->position.z += 1.0;
        monkey->position.y += 1.0;
        //monkey->parent = mirrorCube.get();
        monkey->scale *= 0.5f;
        monkey->name = "Monkey2";
        add(monkey);


        auto manifoldPortalEnt = std::make_shared<MeshObject>();
        manifoldPortalEnt->loadOBJ("models/quad/quad.obj");
        manifoldPortalEnt->angle.x = 0.0f;
        manifoldPortalEnt->position.z = 1.0f;
        manifoldPortalEnt->position.x = -11.5f;
        manifoldPortalEnt->position.y = 1.5f;
        manifoldPortalEnt->instanceType = 2;
        manifoldPortalEnt->name = "Dode Portal Ent";
        portals.push_back(manifoldPortalEnt);
        add(manifoldPortalEnt);

        auto manifoldPortalExit = std::make_shared<MeshObject>();
        manifoldPortalExit->loadOBJ("models/quad/quad.obj");
        manifoldPortalExit->angle.x = 0.0f;
        manifoldPortalExit->position = dode->position;
        manifoldPortalExit->position.z = 1.0f;
        manifoldPortalExit->position.x += 0.0f;
        manifoldPortalExit->position.y += 0.0f;
        manifoldPortalExit->name = "Dode Portal Exit";
        portals.push_back(manifoldPortalExit);
        add(manifoldPortalExit);

        manifoldPortalEnt->instanceType = 2;
        manifoldPortalExit->instanceType = 2;
        manifoldPortalEnt->portalRef = manifoldPortalExit.get();
        manifoldPortalExit->portalRef = manifoldPortalEnt.get();
        makePortalFrame(manifoldPortalEnt);
        makePortalFrame(manifoldPortalExit);

        auto portalLabel = makeTextLabel(manifoldPortalEnt, "textures/fyp_dode.png");
    }

    {
        auto hallwayEnt = std::make_shared<MeshObject>();
        hallwayEnt->loadOBJ("models/quad/quad.obj");
        hallwayEnt->angle.x = 0.0f;
        hallwayEnt->position.z = 1.0f;
        hallwayEnt->position.x = 7.0f;
        hallwayEnt->position.y = 6.0f;
        hallwayEnt->instanceType = 5;
        hallwayEnt->name = "Hallway Ent";
        add(hallwayEnt);

        auto hallwayLabel = makeTextLabel(hallwayEnt, "textures/fyp_hallway.png");

        auto hallwayExit = std::make_shared<MeshObject>();
        hallwayExit->loadOBJ("models/quad/quad.obj");
        hallwayExit->angle.x = 0.0f;
        hallwayExit->position.z = 1.0f;
        hallwayExit->position.x = 7.0f;
        hallwayExit->position.y = 10.0f;
        hallwayExit->instanceType = 5;
        hallwayExit->name = "Hallway Exit";
        add(hallwayExit);

        auto hallL = std::make_shared<MeshObject>();
        hallL->loadOBJ("models/quad/quad.obj");
        hallL->angle.x = 0.0f;
        hallL->angle.z += 90.0f;
        hallL->scale.x *= 2.0f;
        hallL->position.z = 1.0f;
        hallL->position.x = 8.0f;
        hallL->position.y = 8.0f;
        hallL->name = "Hallway L Wall";
        add(hallL);

        auto hallR = std::make_shared<MeshObject>();
        hallR->loadOBJ("models/quad/quad.obj");
        hallR->angle.x = 0.0f;
        hallR->angle.z += 90.0f;
        hallR->scale.x *= 2.0f;
        hallR->position.z = 1.0f;
        hallR->position.x = 6.0f;
        hallR->position.y = 8.0f;
        hallR->name = "Hallway R Wall";
        add(hallR);

        auto hallU = std::make_shared<MeshObject>();
        hallU->loadOBJ("models/quad/quad.obj");
        hallU->angle.x = 90.0f;
        hallU->angle.z += 90.0f;
        hallU->scale.x *= 2.0f;
        hallU->position.z = 2.0f;
        hallU->position.x = 7.0f;
        hallU->position.y = 8.0f;
        hallU->name = "Hallway Ceiling";
        add(hallU);

        hallwayBounds.position = { 7.0f, 8.0f, 2.0f };
        hallwayBounds.size = { 2.0f, 4.0f, 2.0f };
    }



    //auto obj3 = std::make_shared<MeshObject>();
    //obj3->loadOBJ("models/room/room.obj");
    //obj3->scale *= 5.0f;
    //obj3->position.x = 3.0f;
    //add(obj3);

    {
        auto floor = std::make_shared<MeshObject>();
        floor->loadQuad("textures/checkerboard.png", 30.0f);
        floor->angle.x = 0;
        floor->scale *= 30.0f;
        //floor->position.z = 0.5f;
        //floor->position.y = 0.5f;
        add(floor);
    }
    {
        /*
        auto wallB = std::make_shared<MeshObject>();
        wallB->loadQuad("textures/243.jpg", 30.0f, { {1.0, 1.0, 1.0},{1.0, 1.0, 1.0},{1.0, 1.0, 1.0},{1.0, 1.0, 1.0} });
        wallB->scale *= 30.0f;
       
        wallB->position.y = -15.0f;
        add(wallB);
        auto wallF = std::make_shared<MeshObject>();
        wallF->loadQuad("textures/243.jpg", 30.0f, { {1.0, 1.0, 1.0},{1.0, 1.0, 1.0},{1.0, 1.0, 1.0},{1.0, 1.0, 1.0} });
        wallF->scale *= 30.0f;
        wallF->scale.y *= -1;
        wallF->position.y = 15.0f;
        add(wallF);*/
    }


    auto obj4 = std::make_shared<MeshObject>();
    obj4->loadOBJ("models/cubeInv/cube.obj");
    obj4->scale *= 10.0f;
    obj4->scale.x *= 2.0;
    obj4->scale.z *= 2.0;
    obj4->position.z += 9.9f;
    obj4->name = "room";
    add(obj4);


    {
        //create rotated portals
        auto portal = std::make_shared<MeshObject>();
        portal->loadOBJ("models/quad/quad.obj");
        portal->angle = {45, 45, 135};
        portal->position.z = 3.0f;
        portal->position.x = 7.5f;
        portal->position.y = -2.5f;
        portal->instanceType = 2; //set type to be an actual portal
        portal->name = "Portal 1";
        portals.push_back(portal);
        add(portal);

        auto portal2 = std::make_shared<MeshObject>();
        portal2->loadOBJ("models/quad/quad.obj");
        portal2->angle = { 45, 45, 135 };
        portal2->position.z = 1.2f;
        portal2->position.x = 10.5f;
        portal2->position.y = -1.5f;
        portal2->name = "Portal 2";
        portals.push_back(portal2);
        add(portal2);

        portal->instanceType = 2;
        portal2->instanceType = 2;
        portal->portalRef = portal2.get();
        portal2->portalRef = portal.get();
        makePortalFrame(portal);
        makePortalFrame(portal2);

        auto portalLabel2 = makeTextLabel(portal, "textures/fyp_portalsRotated.png");
        portalLabel2->angle.x = 120;
        portalLabel2->angle.z = -70;
    }

    {
        //create rotated portals
        auto portal = std::make_shared<MeshObject>();
        portal->loadOBJ("models/quad/quad.obj");
        portal->angle.x = 5.0f;
        portal->position.z = 1.0f;
        portal->position.x = 9.0;
        portal->position.y = 0.0f;
        portal->instanceType = 2; //set type to be an actual portal
        portal->name = "Portal 1";
        portals.push_back(portal);
        add(portal);

        auto portal2 = std::make_shared<MeshObject>();
        portal2->loadOBJ("models/quad/quad.obj");
        portal2->angle.x = -5.0f;
        portal2->position.z = 1.0f;
        portal2->position.x = 9.0;
        portal2->position.y = -4.0f;
        portal2->name = "Portal 2";
        portals.push_back(portal2);
        add(portal2);

        portal->instanceType = 2;
        portal2->instanceType = 2;
        portal->portalRef = portal2.get();
        portal2->portalRef = portal.get();
        makePortalFrame(portal);
        makePortalFrame(portal2);
    }


    {
        //create rotated portals
        auto portal = std::make_shared<MeshObject>();
        portal->loadOBJ("models/quad/quad.obj");
        portal->angle.x = 0.0f;
        portal->position.z = 1.0f;
        portal->position.x = 9.0;
        portal->position.y = -9.0f;
        portal->instanceType = 2; //set type to be an actual portal
        portal->name = "Portal 1";
        portals.push_back(portal);
        add(portal);

        auto portal2 = std::make_shared<MeshObject>();
        portal2->loadOBJ("models/quad/quad.obj");
        portal2->angle.x = 0.0f;
        portal2->position.z = 0.5f;
        portal2->position.x = 9.0;
        portal2->position.y = -10.0f;
        portal2->scale *= 0.5;
        portal2->name = "Portal 2";
        portals.push_back(portal2);
        add(portal2);

        portal->instanceType = 2;
        portal2->instanceType = 2;
        portal->portalRef = portal2.get();
        portal2->portalRef = portal.get();
        makePortalFrame(portal);
        makePortalFrame(portal2);

        auto portalLabel2 = makeTextLabel(portal, "textures/fyp_portalsScaled.png");
        portalLabel2->angle.z = -90;
    }
    
    
    
   
}

void Scene::update(float dt)
{
    camera->update(dt);
    for (auto& obj : objects)
        obj->update(dt);

    for (auto& obj : portals) //portal teleport checks
    {
        if (obj->instanceType == 2) //regular portal
        {
            updatePortal(obj);
        }
        else
        {
            updateManifoldPortal(obj);
        }
    }

    if (hallwayBounds.pointOverlaps(camera->eye)) //scale movement speed when inside hallway
    {
        glm::vec3 scaledSpeed = glm::vec3(1.0, 1.0, 1.0) / glm::vec3(tlasStretch);
        camera->cameraSpeed = scaledSpeed;
    }
    else
    {
        camera->cameraSpeed = { 1.0, 1.0, 1.0 };
    }

    showObjectManipulator(dt);
}

void Scene::updatePortal(std::shared_ptr<BaseObject> obj)
{
    obj->obb.position = obj->position;
    obj->obb.size = obj->scale * glm::vec3{ 2.0, 1.0, 2.0 };
    bool curFront = obj->inFrontOfPortal;

    obj->inFrontOfPortal = obj->obb.isInFrontY(camera->eye, obj->angle); //use an obb check to see which side of the portal the camera is on
    if (obj->wasCollidingWithPlayer)
    {
        if (curFront != obj->inFrontOfPortal)
        {
            //teleport
            std::cout << "teleport!" << std::endl;

            auto portal = obj->portalRef;

            //calculate rotation
            auto orientation = glm::quat(glm::vec3(glm::radians(obj->angle.x), glm::radians(obj->angle.y), glm::radians(obj->angle.z)));
            auto portalOri = glm::quat(glm::vec3(glm::radians(portal->angle.x), glm::radians(portal->angle.y), glm::radians(portal->angle.z)));
            auto finalOri = glm::inverse(orientation) * portalOri;

            //tilt
            float tilt = std::fmod(portal->angle.y - obj->angle.y, 360.0f);
            camera->cameraAngle.z += glm::radians(tilt);

            //rotate around the edge of the portal
            glm::vec3 distance = camera->eye - obj->position;
            glm::vec3 rotatedDistance = glm::mat4(finalOri) * glm::scale(portal->scale / obj->scale) * glm::vec4(distance, 1.0);

            //offset xyz
            camera->eye += obj->portalRef->position - obj->position;

            //change world scale for scaled portals
            camera->worldScale *= obj->scale; 
            camera->worldScale /= portal->scale;
            

            //rotate xyz based on portal angle
            camera->eye -= distance;
            camera->eye += rotatedDistance;

            //dont fall through ground
            camera->eye.z = 1.5f / camera->worldScale.z;

            //rotate camera based on portal angle
            camera->cameraAngle += 
                glm::vec3(glm::radians(portal->angle.z), glm::radians(portal->angle.x), glm::radians(portal->angle.y)) - 
                glm::vec3(glm::radians(obj->angle.z), glm::radians(obj->angle.x), glm::radians(obj->angle.y));

            camera->updateEyePosition();
        }
    }

    if (obj->obb.pointOverlaps(camera->eye, obj->angle))
    {
        obj->wasCollidingWithPlayer = true;
    }
    else
    {
        obj->wasCollidingWithPlayer = false;
    }
}
void Scene::updateManifoldPortal(std::shared_ptr<BaseObject> obj)
{
    AABB boxBounds = {};
    boxBounds.position = obj->position;
    boxBounds.size = obj->scale * 2.0f;

    if (obj->wasCollidingWithPlayer)
    {
        //std::cout << "inside!" << camera->eye.x << std::endl;
        if (!boxBounds.pointOverlaps(camera->eye)) //no longer overlapping so teleport?
        {
            if (glm::length(boxBounds.position - camera->eye) < boxBounds.size.x * 2.0) //if near area (so portals still work)
            {
                //adjust position to other side
                if ((boxBounds.position.x - camera->eye.x) * 2 > (boxBounds.size.x))
                {
                    if (obj->instanceType == 3) //torus
                        camera->eye.x += boxBounds.size.x;
                    else //assume its mirrored
                    {
                        //flip to match mirrored world
                        camera->scale.x *= -1;
                        camera->cameraAngle.x += glm::radians(180.0f); 
                        camera->cameraAngle.x *= -1;
                    }
                }
                else if ((boxBounds.position.x - camera->eye.x) * 2 < -(boxBounds.size.x))
                {
                    if (obj->instanceType == 3)
                        camera->eye.x -= boxBounds.size.x;
                    else
                    {
                        camera->scale.x *= -1;
                        camera->cameraAngle.x += glm::radians(180.0f);
                        camera->cameraAngle.x *= -1;
                    }
                }
                else if ((boxBounds.position.y - camera->eye.y) * 2 > (boxBounds.size.y))
                {
                    if (obj->instanceType == 3)
                        camera->eye.y += boxBounds.size.y;
                    else
                    {
                        camera->scale.z *= -1;
                        camera->cameraAngle.x += glm::radians(180.0f);
                        camera->cameraAngle.x *= -1;
                    }

                }
                else if ((boxBounds.position.y - camera->eye.y) * 2 < -(boxBounds.size.y))
                {
                    if (obj->instanceType == 3)
                        camera->eye.y -= boxBounds.size.y;
                    else
                    {
                        camera->scale.z *= -1;
                        camera->cameraAngle.x += glm::radians(180.0f);
                        camera->cameraAngle.x *= -1;
                    }
                }
            }

        }
    }

    if (boxBounds.pointOverlaps(camera->eye))
        obj->wasCollidingWithPlayer = true;
    else
        obj->wasCollidingWithPlayer = false;
}