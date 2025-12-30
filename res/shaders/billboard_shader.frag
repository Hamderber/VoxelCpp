#version 460

layout (location = 0) in vec2 fragOffset;

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
	vec4 position;
	vec4 color;
	float radius;
} push;

void main()
{
	// Make test billboard a circle r = 1
	if (sqrt(dot(fragOffset, fragOffset)) >= 1.0) discard;

	outColor = vec4(push.color.xyz, 1.0);
}