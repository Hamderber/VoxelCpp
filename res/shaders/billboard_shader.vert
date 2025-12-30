#version 460
#extension GL_KHR_vulkan_glsl : enable

// Hardcoded billboard test
const vec2 OFFSETS[6] = vec2[](
  vec2(-1.0, -1.0),
  vec2(-1.0, 1.0),
  vec2(1.0, -1.0),
  vec2(1.0, -1.0),
  vec2(-1.0, 1.0),
  vec2(1.0, 1.0)
);

layout (location = 0) out vec2 fragOffset;

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
	vec4 position;
	vec4 color;
	float radius;
} push;

void main()
{
	fragOffset = OFFSETS[gl_VertexIndex];
	vec3 cameraRightWorld = { globalUbo.viewMatrix[0][0], globalUbo.viewMatrix[1][0], globalUbo.viewMatrix[2][0] };
	vec3 cameraUpWorld = { globalUbo.viewMatrix[0][1], globalUbo.viewMatrix[1][1], globalUbo.viewMatrix[2][1] };

	// Billboard at light postion
	vec3 positionWorld = push.position.xyz
		+ push.radius * fragOffset.x * cameraRightWorld
		+ push.radius * fragOffset.y * cameraUpWorld;

	gl_Position = globalUbo.projectionMatrix * globalUbo.viewMatrix * vec4(positionWorld, 1.0);
}