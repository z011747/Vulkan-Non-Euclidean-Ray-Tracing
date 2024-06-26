#version 460
#extension GL_EXT_ray_tracing : enable

#include "rayCommon.glsl"
layout(location = 0) rayPayloadInEXT Ray ray;

void main()
{
    ray.hitValue = vec3(0.0, 0.0, 0.2);
}