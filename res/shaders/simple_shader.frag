#version 460

layout (location = 0) in vec3 fragColor;
layout (location = 1) in vec3 fragPosWorld;
layout (location = 2) in vec3 fragNormalWorld;

layout (location = 0) out vec4 outColor;

layout (set = 0, binding = 0) uniform GlobalUbo
{
	mat4 projectionView;
	vec4 ambientLightColor;
	vec3 lightPosition;
	// same as uint32_t
	uint paddingUnused;
	vec4 lightColor;
} globalUbo;

layout (push_constant) uniform Push
{
	mat4 modelMatrix;
} push;

void main()
{
	vec3 dirToLight = globalUbo.lightPosition - fragPosWorld;
	// Inverse square law
	float attenuation = 1.0 / dot(dirToLight, dirToLight);

	// w is intensity for light vec4s
	vec3 lightColor = globalUbo.lightColor.xyz * globalUbo.lightColor.w * attenuation;
	vec3 ambientLight = globalUbo.ambientLightColor.xyz * globalUbo.ambientLightColor.w;
	vec3 diffuseLight = lightColor * max(dot(normalize(fragNormalWorld), normalize(dirToLight)), 0);

	outColor = vec4((diffuseLight + ambientLight) * fragColor, 1.0);
}