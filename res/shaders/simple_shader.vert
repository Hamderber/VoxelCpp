#version 450

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;
layout (location = 2) in vec3 normal;
layout (location = 3) in vec2 uv;

layout (location = 0) out vec3 fragColor;

layout (push_constant) uniform Push
{
	// Projection * View * Model
	mat4 transform;
	mat4 modelMatrix;
} push;

const vec3 DIR_TO_LIGHT = normalize(vec3(1.0, -3.0, -1.0));
const float AMBIENT = 0.02;

void main()
{
	gl_Position = push.transform * vec4(position, 1.0);
	
	// Uniform object scaling option (cheap)
	vec3 normalWorldSpace = normalize(mat3(push.modelMatrix) * normal);
	
	// Non-uniform object scaling option
	//mat3 normalMatrix = transpose(inverse(mat3(push.modelMatrix)));
	//vec3 normalWorldSpace = normalize(normalMatrix * normal);

	float lightIntensity = AMBIENT + max(dot(normalWorldSpace, DIR_TO_LIGHT), 0);

	fragColor = color * lightIntensity;
}