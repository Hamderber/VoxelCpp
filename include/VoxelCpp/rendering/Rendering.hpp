#pragma once
#include <VoxelCpp/rendering/Pipeline.hpp>
#include <VoxelCpp/rendering/Device.hpp>
#include <VoxelCpp/rendering/Window.hpp>
#include <VoxelCpp/rendering/Swapchain.hpp>
#include <VoxelCpp/game/GameObject.hpp>
#include <memory>
#include <vulkan/vulkan_core.h>
#include <vector>
#include <cstdint>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/fwd.hpp>

namespace App { class App; }

namespace Rendering
{
	// TEMPORARY
	struct SimplePushConstantData
	{
		glm::mat2 transform{ 1 };
		glm::vec2 offset;
		alignas(16) glm::vec3 color;
	};

	class Rendering
	{
	public:
		Rendering(App::App &rApp);
		~Rendering();

		Rendering(const Rendering &) = delete;
		Rendering &operator=(const Rendering &) = delete;

		void loop();

		Window window;

	private:
		void load_game_objects();
		void wait_idle();
		void create_pipeline_layout();
		void create_pipeline();
		void create_command_buffers();
		void destroy_command_buffers();
		void draw_frame();
		void recreate_swapchain();
		void record_command_buffer(uint32_t imageIndex);
		void render_game_objects(VkCommandBuffer commandBuffer);

		App::App &m_rApp;
		Device m_device;
		std::unique_ptr<Swapchain> m_pSwapchain;
		std::unique_ptr<Pipeline> m_pPipeline;
		VkPipelineLayout m_pPipelineLayout;
		std::vector<VkCommandBuffer> m_vCommandBuffers;
		std::vector<Game::GameObject> m_vGameObjects;
	};
}