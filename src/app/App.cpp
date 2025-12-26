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
#include <chrono>
#include <VoxelCpp/input/KeyboardMovementController.hpp>
#include <glm/glm.hpp>
#include <glm/fwd.hpp>
#include <glm/detail/compute_common.hpp>

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
		auto root = ProgramConstants::root_filepath();

		std::shared_ptr<Rendering::Model> pModel =
			Rendering::Model::create_model_from_file(m_device, (root / "res/models/smooth_vase.obj").generic_string());

		auto gameObject = Game::GameObject::create();
		gameObject.pModel = pModel;
		gameObject.transform.translation = { 0.0f, 0.0f, 2.5f };
		gameObject.transform.scale = { 0.5f, 0.5f, 0.5f };

		m_vGameObjects.push_back(std::move(gameObject));
	}

	void App::loop(void)
	{
		// TODO: Make this not a complete mess

		Rendering::RenderSystem renderSystem{ m_device, m_renderer.swapchain_renderpass_get() };
		
		Rendering::Camera camera{};

		auto cameraObject = Game::GameObject::create();
		Input::KeyboardMovementController cameraController{};

		// Call this last for accurate timestamps
		auto currentTime = std::chrono::high_resolution_clock::now();

		while (!m_window.should_close())
		{
			glfwPollEvents();

			auto newTime = std::chrono::high_resolution_clock::now();
			float dt = std::chrono::duration<float, std::ratio<1>>(newTime - currentTime).count();
			dt = glm::min(dt, ProgramConstants::MAX_FRAME_TIME_SEC);
			currentTime = newTime;

			cameraController.move_in_plane_xz(m_window.GLFWwindow_get(), cameraObject, dt);
			camera.set_view_yxz(cameraObject.transform.translation, cameraObject.transform.eulerRotationRadians);
			camera.projection_perspective_set(glm::radians(50.f), m_renderer.aspect_ratio(), 0.1f, 10.f);

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