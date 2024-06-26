#version 460
#extension GL_EXT_ray_tracing : enable
#extension GL_GOOGLE_include_directive : enable
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_buffer_reference2 : enable
#extension GL_EXT_scalar_block_layout : enable
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : enable

layout(binding = 0, set = 0) uniform accelerationStructureEXT TLAS;
hitAttributeEXT vec2 attribs;

#include "rayCommon.glsl"
layout(location = 0) rayPayloadInEXT Ray ray;
layout(location = 1) rayPayloadEXT bool shadowed;


#include "Triangles.glsl"
#include "UniformBuffer.glsl"

layout(binding = 5) uniform sampler2D[] image;

vec3 getDiffuse(vec3 light, vec3 normal, vec3 diffuseCol, vec3 ambientCol)
{
    vec3 dif = max(dot(light, normal), 0.0) * diffuseCol;
    dif += ambientCol;
    return dif;
}

void main()
{
    const vec3 barycentricCoords = vec3(1.0f - attribs.x - attribs.y, attribs.x, attribs.y);
    //get triangle data for uv and normal data
    Triangle tri = unpackTriangle(gl_PrimitiveID, instancesBlock.instance[gl_InstanceID].vertices, instancesBlock.instance[gl_InstanceID].indices);

    //scale to match the hallway
    vec3 stretchScale = vec3(1.0);
    if (ray.targetTLAS == 1)
    stretchScale *= ubo.stretch.xyz;

 
  
    vec3 normal = normalize(tri.vertices[0].normal * barycentricCoords.x + tri.vertices[1].normal * barycentricCoords.y + tri.vertices[2].normal * barycentricCoords.z);
    normal = normalize(vec3(normal * gl_WorldToObjectEXT));
    vec3 pixelPos = tri.vertices[0].pos * barycentricCoords.x + tri.vertices[1].pos * barycentricCoords.y + tri.vertices[2].pos * barycentricCoords.z;
    pixelPos = vec3(gl_ObjectToWorldEXT * vec4(pixelPos, 1.0));

    //point light calculations
    vec3 lightPos = ubo.light.pos.xyz;
    vec3 lightDir = normalize(lightPos);
    float lightDistance = length(lightPos - (pixelPos / stretchScale));
    float lightIntensity = 80.0f / (lightDistance*lightDistance);
    vec3 diffuseDir = normalize(lightPos - (pixelPos / stretchScale));
	
    vec3 diffuse = getDiffuse(diffuseDir, normal, texture(image[gl_InstanceCustomIndexEXT], tri.uv).rgb * tri.color, vec3(0.01));
    float attenuation = 1.0f;

    bool castShadow = true;
    if (castShadow)
    {
        float tmin = 0.001;
	    float tmax = lightDistance;
	    vec3 origin = gl_WorldRayOriginEXT + gl_WorldRayDirectionEXT * gl_HitTEXT;
	    shadowed = true;
        traceRayEXT(TLAS, gl_RayFlagsTerminateOnFirstHitEXT | gl_RayFlagsOpaqueEXT | gl_RayFlagsSkipClosestHitShaderEXT, 0xFF, 0, 0, 0, origin, tmin, lightDir, tmax, 1);
    }

    if (shadowed) 
    {
        attenuation = 0.3f;
    }

    ray.distanceTraveled += length(ray.rayOrigin - pixelPos);


    //check for specific object properties
    uint type = instancesBlock.instance[gl_InstanceID].type;
    if (type == 1) //reflective object
    {
        ray.done = 0; //reset to 0 to reshoot
        ray.rayOrigin = pixelPos;
        ray.rayDir = reflect(gl_WorldRayDirectionEXT, normal);
    }
    else if (type == 2) //portal
    {
        ray.done = 0; //reset to 0 to reshoot

        vec3 p = tri.vertices[0].pos * barycentricCoords.x + tri.vertices[1].pos * barycentricCoords.y + tri.vertices[2].pos * barycentricCoords.z;
        vec4 pRotated = instancesBlock.instance[gl_InstanceID].portalRotation * vec4(p, 1.0);
        p = vec3(gl_ObjectToWorldEXT * pRotated); //now rotate to world

        ray.rayOrigin = p + instancesBlock.instance[gl_InstanceID].portalTranslation.xyz; //now offset

        vec4 dir = instancesBlock.instance[gl_InstanceID].portalRotation * vec4(gl_WorldRayDirectionEXT, 1.0);
        ray.rayDir = dir.xyz;

        ray.rayOrigin += ray.rayDir * 0.01; //offset slightly
    }
    else if (type == 3) //torus (repeating geometry)
    {
        ray.done = 0; //reset to 0 to reshoot
        ray.rayOrigin = pixelPos + (normal*2.0*3.0); //offset by normal
    }
    else if (type == 4)
    {
        
    }
    else if (type == 5) //switch to stretched tlas (hallway)
    {
        ray.done = 0; //reset to 0 to reshoot
    
        if (ray.targetTLAS == 0) //stretch
        {
            ray.rayOrigin = pixelPos * ubo.stretch.xyz;
            ray.targetTLAS = 1;
        }
        else //unstretch
        {
            ray.rayOrigin = pixelPos / ubo.stretch.xyz;
            ray.targetTLAS = 0;
        }
    }
    ray.hitValue = vec3(attenuation * (diffuse /*+ specular*/));
}
