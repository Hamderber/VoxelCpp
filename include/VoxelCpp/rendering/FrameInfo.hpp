#pragma once
#include <vulkan/vulkan_core.h>
#include <VoxelCpp/game/GameObject.hpp>
#include <cstdint>
#include <glm/fwd.hpp>

namespace Rendering { class Camera; }

namespace Rendering
{
	// TEMPORARY
	constexpr size_t MAX_LIGHTS = 10;

	struct PointLight
	{
		glm::vec4 positon{};
		glm::vec4 color{};
	};

	// Note: W for light vec4s is intensity
	struct GlobalUBO
	{
		glm::mat4 projectionMatrix{ 1.f };
		glm::mat4 viewMatrix{ 1.f };
		glm::mat4 inverseViewMatrix{ 1.f };
		glm::vec4 ambientLightColor{ 1.f, 1.f, 1.f, .02f };
		glm::vec3 lightPosition{ 1.f };
		// Potentially usable future data since this spacing is required for std140 compliance
		uint32_t paddingUnused;
		PointLight pPointLights[MAX_LIGHTS]{};
		uint32_t lightCount{};
	};

	struct FrameInfo
	{
		int frameIndex;
		float frame_dt;
		VkCommandBuffer commandBuffer;
		Camera &rCamera;
		VkDescriptorSet globalDescSet;
		Game::GameObject::Map &rGameObjects;
	};
}