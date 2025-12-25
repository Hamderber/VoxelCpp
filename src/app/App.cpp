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
#include <VoxelCpp/rendering/Camera.hpp>

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

	// Temporary
	std::unique_ptr<Rendering::Model> createCubeModel(Rendering::Device &device, glm::vec3 offset)
	{
		std::vector<Rendering::Model::Vertex> vertices{

			// left face (white)
			{{-.5f, -.5f, -.5f}, {.9f, .9f, .9f}},
			{{-.5f, .5f, .5f}, {.9f, .9f, .9f}},
			{{-.5f, -.5f, .5f}, {.9f, .9f, .9f}},
			{{-.5f, -.5f, -.5f}, {.9f, .9f, .9f}},
			{{-.5f, .5f, -.5f}, {.9f, .9f, .9f}},
			{{-.5f, .5f, .5f}, {.9f, .9f, .9f}},

			// right face (yellow)
			{{.5f, -.5f, -.5f}, {.8f, .8f, .1f}},
			{{.5f, .5f, .5f}, {.8f, .8f, .1f}},
			{{.5f, -.5f, .5f}, {.8f, .8f, .1f}},
			{{.5f, -.5f, -.5f}, {.8f, .8f, .1f}},
			{{.5f, .5f, -.5f}, {.8f, .8f, .1f}},
			{{.5f, .5f, .5f}, {.8f, .8f, .1f}},

			// top face (orange, remember y axis points down)
			{{-.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
			{{.5f, -.5f, .5f}, {.9f, .6f, .1f}},
			{{-.5f, -.5f, .5f}, {.9f, .6f, .1f}},
			{{-.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
			{{.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
			{{.5f, -.5f, .5f}, {.9f, .6f, .1f}},

			// bottom face (red)
			{{-.5f, .5f, -.5f}, {.8f, .1f, .1f}},
			{{.5f, .5f, .5f}, {.8f, .1f, .1f}},
			{{-.5f, .5f, .5f}, {.8f, .1f, .1f}},
			{{-.5f, .5f, -.5f}, {.8f, .1f, .1f}},
			{{.5f, .5f, -.5f}, {.8f, .1f, .1f}},
			{{.5f, .5f, .5f}, {.8f, .1f, .1f}},

			// nose face (blue)
			{{-.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
			{{.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
			{{-.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
			{{-.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
			{{.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
			{{.5f, .5f, 0.5f}, {.1f, .1f, .8f}},

			// tail face (green)
			{{-.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
			{{.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
			{{-.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
			{{-.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
			{{.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
			{{.5f, .5f, -0.5f}, {.1f, .8f, .1f}},

		};
		for (auto &v : vertices)
		{
			v.position += offset;
		}
		return std::make_unique<Rendering::Model>(device, vertices);
	}

	void App::game_objects_load()
	{
		std::shared_ptr<Rendering::Model> pModel = createCubeModel(m_device, {});

		auto cube = Game::GameObject::create();
		cube.pModel = pModel;
		cube.transform.translation = { 0.0f, 0.0f, 2.5f };
		cube.transform.scale = { 0.5f, 0.5f, 0.5f };

		m_vGameObjects.push_back(std::move(cube));
	}

	void App::loop(void)
	{
		Rendering::RenderSystem renderSystem{ m_device, m_renderer.swapchain_renderpass_get() };
		
		Rendering::Camera camera{};
		camera.set_view_direction(glm::vec3(0), glm::vec3(0.5f, 0, 1));
		//camera.projection_perspective_set(glm::radians(50.f), m_renderer.aspect_ratio(), 0.1f, 10.0f);
		camera.set_view_target(glm::vec3(-1.f, -2.f, 2.f), glm::vec3(0.f, 0.f, 2.5f));

		while (!m_window.should_close())
		{
			glfwPollEvents();

			if (auto commandBuffer = m_renderer.frame_begin())
			{
				m_renderer.swapchain_renderpass_begin(commandBuffer);
				renderSystem.game_objects_render(commandBuffer, m_vGameObjects, camera);
				m_renderer.swapchain_renderpass_end(commandBuffer);
				m_renderer.frame_end();
			}
		}

		m_renderer.wait_idle();
	}
}