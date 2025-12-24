#pragma once
#include <vulkan/vulkan_core.h>
#include <vector>
#include <memory>

namespace Game { class GameObject; }
namespace Rendering { class Device; class Pipeline; }

namespace Rendering
{
	class RenderSystem
	{
	public:
		RenderSystem(Device &rDevice, VkRenderPass renderPass);
		~RenderSystem();

		RenderSystem(const RenderSystem &) = delete;
		RenderSystem &operator=(const RenderSystem &) = delete;

		void game_objects_render(VkCommandBuffer commandBuffer, std::vector<Game::GameObject> &vGameObjects);

	private:
		void pipeline_layout_create();
		void pipeline_create(VkRenderPass renderPass);
		void pipeline_destroy();

		Device &m_rDevice;
		std::unique_ptr<Pipeline> m_pPipeline;
		VkPipelineLayout m_pipelineLayout;
	};
}