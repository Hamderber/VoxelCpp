#version 450

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;
layout (location = 2) in vec3 normal;
layout (location = 3) in vec2 uv;

layout (location = 0) out vec3 fragColor;
layout (location = 1) out vec3 fragPosWorld;
layout (location = 2) out vec3 fragNormalWorld;

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
	vec4 positionWorld = push.modelMatrix * vec4(position, 1.0);
	gl_Position =  globalUbo.projectionView * positionWorld;
	
	fragNormalWorld = normalize(mat3(push.modelMatrix) * normal);
	fragPosWorld = positionWorld.xyz;
	fragColor = color;
}