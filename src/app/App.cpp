#include <filesystem>
#include <VoxelCpp/app/App.hpp>
#include <ksc_log.hpp>
#include <VoxelCpp/rendering/Renderer.hpp>
#include <VoxelCpp/rendering/Window.hpp>
#include <memory>
#include <glm/gtc/constants.hpp>
#include <cassert>
#include <VoxelCpp/game/GameObject.hpp>
#include <VoxelCpp/rendering/Model.hpp>
#include <VoxelCpp/rendering/Pipeline.hpp>
#include <cstdint>
#include <stdexcept>
#include <utility>
#include <vector>
#include <glm/gtc/constants.hpp>
#include <vulkan/vulkan_core.h>
#include <VoxelCpp/rendering/RenderSystem.hpp>

namespace App
{
	App::App(std::filesystem::path root) : m_root{root}
	{
		game_objects_load();

		ksc_log::debug("App created. Root: " + m_root.generic_string());
	}

	App::~App()
	{
		ksc_log::debug("Destroying app...");
	}

	void App::game_objects_load()
	{
		// Hello world triangle (model version)
		std::vector<Rendering::Model::Vertex> vVerticies{
			{{ 0.0f, -0.5f }, {1.0f, 0, 0}},
			{{ 0.5f, 0.5f }, {0, 1.0f, 0}},
			{{ -0.5f, 0.5f }, {0, 0, 1.0f}}
		};

		auto pModel = std::make_shared<Rendering::Model>(m_device, vVerticies);

		auto triangleGO = Game::GameObject::create();
		triangleGO.pModel = pModel;
		triangleGO.color = { 0.1f, 0.8f, 0.1f };
		triangleGO.transform2D.translation.x = 0.2f;
		triangleGO.transform2D.scale = { 2, 0.5f };
		triangleGO.transform2D.rotationRadians = 0.0025f * glm::two_pi<float>();

		m_vGameObjects.push_back(std::move(triangleGO));
	}

	void App::loop(void)
	{
		Rendering::RenderSystem renderSystem{ m_device, m_renderer.swapchain_renderpass_get() };

		while (!m_window.should_close())
		{
			glfwPollEvents();

			if (auto commandBuffer = m_renderer.frame_begin())
			{
				m_renderer.swapchain_renderpass_begin(commandBuffer);
				renderSystem.game_objects_render(commandBuffer, m_vGameObjects);
				m_renderer.swapchain_renderpass_end(commandBuffer);
				m_renderer.frame_end();
			}
		}

		m_renderer.wait_idle();
	}
}