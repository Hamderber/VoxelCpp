#pragma once
#include <memory>
#include <filesystem>
#include <VoxelCpp/rendering/Renderer.hpp>
#include "ProgramConstants.hpp"
#include <VoxelCpp/rendering/Device.hpp>
#include <VoxelCpp/rendering/Window.hpp>
#include <VoxelCpp/rendering/Pipeline.hpp>
#include <vulkan/vulkan_core.h>
#include <vector>
#include <VoxelCpp/game/GameObject.hpp>

namespace App
{
	class App
	{
	public:
		App(std::filesystem::path root);
		~App();

		App(const App &) = delete;
		App &operator=(const App &) = delete;

		void loop();

	private:

		// TODO: Encapsulate in class(es)
		void game_objects_load();
		void pipeline_layout_create();
		void pipeline_layout_destroy();
		void pipeline_create();
		void game_objects_render(VkCommandBuffer commandBuffer);

		// TODO: Encapsulate in a class
		Rendering::Window m_window{ DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT, APPLICATION_NAME };
		Rendering::Device m_device{ m_window };
		Rendering::Renderer m_renderer{ m_window, m_device };

		std::unique_ptr<Rendering::Pipeline> m_pPipeline;
		VkPipelineLayout m_pipelineLayout;
		std::vector<Game::GameObject> m_vGameObjects;

		std::filesystem::path m_root;
	};
}