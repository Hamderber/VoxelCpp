#version 450

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;
layout (location = 2) in vec3 normal;
layout (location = 3) in vec2 uv;

layout (location = 0) out vec3 fragColor;

layout (set = 0, binding = 0) uniform GlobalUbo
{
	mat4 projectionView;
	vec3 lightDirection;
} globalUbo;

layout (push_constant) uniform Push
{
	mat4 modelMatrix;
} push;

const float AMBIENT = 0.02;

void main()
{
	gl_Position = globalUbo.projectionView * push.modelMatrix * vec4(position, 1.0);
	
	// Uniform object scaling option (cheap)
	vec3 normalWorldSpace = normalize(mat3(push.modelMatrix) * normal);
	
	// Non-uniform object scaling option
	//mat3 normalMatrix = transpose(inverse(mat3(push.modelMatrix)));
	//vec3 normalWorldSpace = normalize(normalMatrix * normal);

	float lightIntensity = AMBIENT + max(dot(normalWorldSpace, globalUbo.lightDirection), 0);

	fragColor = color * lightIntensity;
}