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
	mat4 inverseViewMatrix;
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
	vec3 specularLight = vec3(0.0);
	vec3 surfaceNormal = normalize(fragNormalWorld);

	vec3 cameraPosWorld = globalUbo.inverseViewMatrix[3].xyz;
	vec3 viewDirection = normalize(cameraPosWorld - fragPosWorld);

	for (int i = 0; i < globalUbo.lightCount; i++)
	{
		PointLight pointLight = globalUbo.pointLights[i];
		vec3 dirToLight = pointLight.position.xyz - fragPosWorld;
		
		float attenuation = 1.0 / dot(dirToLight, dirToLight);
		dirToLight = normalize(dirToLight);

		// Diffuse
		vec3 intensity = pointLight.color.xyz * pointLight.color.w * attenuation;
		float cosAngleOfIncidence = max(dot(surfaceNormal, dirToLight), 0);
		diffuseLight += intensity * cosAngleOfIncidence;
	
		// Specular
		vec3 halfAngle = normalize(dirToLight + viewDirection);
		float blinnTerm = dot(surfaceNormal, halfAngle);
		blinnTerm = clamp(blinnTerm, 0, 1);
		// TODO: Make exponent a material property for per-object highlight sharpness
		blinnTerm = pow(blinnTerm, 512.0);
		specularLight += intensity * blinnTerm;
	}

	outColor = vec4(diffuseLight * fragColor + specularLight * fragColor, 1.0);
}