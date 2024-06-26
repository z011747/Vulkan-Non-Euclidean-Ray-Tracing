

struct Light
{
    vec4 pos;
    vec4 color;
};



layout(binding = 3, set = 0) uniform MainUniformBuffer 
{
    mat4 view;
    mat4 proj;
	Light light;
    vec4 stretch; //scale of the hallway
    bool insideHallway;
} ubo;