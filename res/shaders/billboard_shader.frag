#version 460

layout (location = 0) in vec2 fragOffset;

layout (location = 0) out vec4 outColor;

layout (set = 0, binding = 0) uniform GlobalUbo
{
	mat4 projectionMatrix;
	mat4 viewMatrix;
	vec4 ambientLightColor;
	vec3 lightPosition;
	// same as uint32_t
	uint paddingUnused;
	vec4 lightColor;
} globalUbo;

void main()
{
	// Make test billboard a circle r = 1
	if (sqrt(dot(fragOffset, fragOffset)) >= 1.0) discard;

	outColor = vec4(globalUbo.lightColor.xyz, 1.0);
}