#pragma once
#include <VoxelCpp/rendering/Pipeline.hpp>
#include <VoxelCpp/rendering/Device.hpp>
#include <VoxelCpp/rendering/Window.hpp>
#include <VoxelCpp/rendering/Swapchain.hpp>
#include <VoxelCpp/rendering/Model.hpp>
#include <memory>
#include <vulkan/vulkan_core.h>
#include <vector>

namespace App { class App; }

namespace Rendering
{
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
		void load_models();
		void wait_idle();
		void create_pipeline_layout();
		void create_pipeline();
		void create_command_buffers();
		void draw_frame();

		App::App &m_rApp;
		Device m_device;
		Swapchain m_swapchain;
		std::unique_ptr<Pipeline> m_pPipeline;
		VkPipelineLayout m_pipelineLayout;
		std::vector<VkCommandBuffer> m_vCommandBuffers;
		std::unique_ptr<Model> m_pModel;
	};
}