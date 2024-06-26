

layout(buffer_reference, scalar) buffer Vertices {float v[]; };
layout(buffer_reference, scalar) buffer Indices {uint i[]; };

struct ObjectInstanceUB 
{
    mat4 portalRotation; //store portal rotation (and scale) based on the 2 portals
    uint64_t vertices; //memory address for calculating uv and normals
    uint64_t indices;
    vec4 portalTranslation; //offset translation between the 2 portals
    uint type; //determines what rays will do to the object
};

struct Vertex 
{
    vec3 pos;
    vec3 color;
    vec2 uv;
    vec3 normal;
    
};

struct Triangle 
{
	Vertex vertices[3];
	vec2 uv;
    vec3 color;
};

layout(std140, binding = 4) uniform InstancesVIUB
{
    ObjectInstanceUB instance[128];
} instancesBlock;



Vertex unpackVertex(uint index, uint64_t vertAddress)
{
    Vertices vertices = Vertices(vertAddress);
    
    //get vertex data from buffer
    uint offset = index*11;
    Vertex vert;
	vert.pos = vec3(vertices.v[offset + 0], vertices.v[offset + 1], vertices.v[offset + 2]);
	vert.color = vec3(vertices.v[offset + 3], vertices.v[offset + 4], vertices.v[offset + 5]);
	vert.uv = vec2(vertices.v[offset + 6], vertices.v[offset + 7]);
    vert.normal = vec3(vertices.v[offset + 8], vertices.v[offset + 9], vertices.v[offset + 10]);
    return vert;
}


Triangle unpackTriangle(uint index, uint64_t vertAddress, uint64_t indexAddress) 
{
	Triangle tri;
	Indices indices = Indices(indexAddress);
    
    //unpack vertices
    tri.vertices[0] = unpackVertex(indices.i[(index*3) + 0], vertAddress);
    tri.vertices[1] = unpackVertex(indices.i[(index*3) + 1], vertAddress);
    tri.vertices[2] = unpackVertex(indices.i[(index*3) + 2], vertAddress);

	// Calculate values at barycentric coordinates
	vec3 barycentricCoords = vec3(1.0f - attribs.x - attribs.y, attribs.x, attribs.y);

    //calculate interpolated uv and color
	tri.uv = tri.vertices[0].uv * barycentricCoords.x + tri.vertices[1].uv * barycentricCoords.y + tri.vertices[2].uv * barycentricCoords.z;
    tri.color = tri.vertices[0].color * barycentricCoords.x + tri.vertices[1].color * barycentricCoords.y + tri.vertices[2].color * barycentricCoords.z;

	return tri;
}
