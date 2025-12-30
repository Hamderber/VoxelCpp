#version 460

layout (location = 0) in vec3 fragColor;
layout (location = 1) in vec3 fragPosWorld;
layout (location = 2) in vec3 fragNormalWorld;

layout (location = 0) out vec4 outColor;

struct PointLight
{
	vec4 position;
	vec4 color;
};

layout (set = 0, binding = 0) uniform GlobalUbo
{
	mat4 projectionMatrix;
	mat4 viewMatrix;
	vec4 ambientLightColor;
	uint paddingUnused;
	// TODO: pass size as specialization constant (or design in a way where this isn't used at all!)
	PointLight pointLights[10];
	uint lightCount;
} globalUbo;

layout (push_constant) uniform Push
{
	mat4 modelMatrix;
} push;

void main()
{
	vec3 diffuseLight = globalUbo.ambientLightColor.xyz * globalUbo.ambientLightColor.w;
	vec3 surfaceNormal = normalize(fragNormalWorld);

	for (int i = 0; i < globalUbo.lightCount; i++)
	{
		PointLight pointLight = globalUbo.pointLights[i];
		vec3 dirToLight = pointLight.position.xyz - fragPosWorld;
		// Inverse square law
		float attenuation = 1.0 / dot(dirToLight, dirToLight);
		vec3 intensity = pointLight.color.xyz * pointLight.color.w * attenuation;
		float cosAngleOfIncidence = max(dot(surfaceNormal, normalize(dirToLight)), 0);
		diffuseLight += intensity * cosAngleOfIncidence;
	}

	outColor = vec4(diffuseLight * fragColor, 1.0);
}