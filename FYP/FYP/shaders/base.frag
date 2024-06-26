#version 450

layout(location = 0) out vec4 outColor;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;

layout(binding = 1) uniform sampler2D texSampler;

void main() {
   outColor = texture(texSampler, fragTexCoord);
   outColor.rgb *= fragColor.rgb;

   outColor.rgb = pow(outColor.rgb, vec3(1.0/2.2));
}