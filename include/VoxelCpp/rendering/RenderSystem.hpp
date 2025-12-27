#pragma once
#include <vulkan/vulkan_core.h>
#include <vector>
#include <memory>
#include <glm/fwd.hpp>

namespace Game { class GameObject; }
namespace Rendering { class Device; class Pipeline; class Camera; }

namespace Rendering
{
	// TEMPORARY
	struct SimplePushConstantData
	{
		glm::mat4 transform{ 1.f };
		glm::mat4 modelMatrix{ 1.f };
	};

	class RenderSystem
	{
	public:
		RenderSystem(Device &rDevice, VkRenderPass renderPass);
		~RenderSystem();

		RenderSystem(const RenderSystem &) = delete;
		RenderSystem &operator=(const RenderSystem &) = delete;

		void game_objects_render(VkCommandBuffer commandBuffer, std::vector<Game::GameObject> &vGameObjects, const Camera &rCAMERA);

	private:
		void pipeline_layout_create();
		void pipeline_create(VkRenderPass renderPass);
		void pipeline_destroy();

		Device &m_rDevice;
		std::unique_ptr<Pipeline> m_pPipeline;
		VkPipelineLayout m_pipelineLayout;
	};
}