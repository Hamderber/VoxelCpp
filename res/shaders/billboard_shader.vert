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

const float RADIUS = 0.1;

void main()
{
	fragOffset = OFFSETS[gl_VertexIndex];
	vec3 cameraRightWorld = { globalUbo.viewMatrix[0][0], globalUbo.viewMatrix[1][0], globalUbo.viewMatrix[2][0] };
	vec3 cameraUpWorld = { globalUbo.viewMatrix[0][1], globalUbo.viewMatrix[1][1], globalUbo.viewMatrix[2][1] };

	// Billboard at light postion
	vec3 positionWorld = globalUbo.lightPosition.xyz
		+ RADIUS * fragOffset.x * cameraRightWorld
		+ RADIUS * fragOffset.y * cameraUpWorld;

	gl_Position = globalUbo.projectionMatrix * globalUbo.viewMatrix * vec4(positionWorld, 1.0);
}