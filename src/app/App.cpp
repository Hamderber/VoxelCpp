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
#include <VoxelCpp/rendering/FrameInfo.hpp>
#include <VoxelCpp/app/ProgramConstants.hpp>
#include <cmath>
#include <VoxelCpp/rendering/BillboardSystem.hpp>

namespace App
{
	App::App(std::filesystem::path root) : m_root{root}
	{
		m_pGlobalDescriptorPool = Rendering::DescriptorPool::Builder(m_device)
			.max_sets_set(Rendering::Swapchain::MAX_FRAMES_IN_FLIGHT)
			.pool_size_add(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, Rendering::Swapchain::MAX_FRAMES_IN_FLIGHT)
			.pool_size_add(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, Rendering::Swapchain::MAX_FRAMES_IN_FLIGHT)
			.build();

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

		std::shared_ptr<Rendering::Model> pCarrierModel = 
			Rendering::Model::create_model_from_file(m_device, (root / "res/models/carrier.obj").generic_string());
		auto carrierObj = Game::GameObject::create();
		carrierObj.pModel = pCarrierModel;
		carrierObj.transform.translation = { 0.f, 0.f, 0.f };
		carrierObj.transform.uniformScale = 1.5f;
		m_mGameObjects.emplace(carrierObj.id(), std::move(carrierObj));

		std::shared_ptr<Rendering::Model> pCubeModel =
			Rendering::Model::create_model_from_file(m_device, (root / "res/models/cube.obj").generic_string());
		auto cubeObj = Game::GameObject::create();
		cubeObj.pModel = pCubeModel;
		cubeObj.transform.translation = { 0.f, -5.f, 0.f };
		cubeObj.transform.uniformScale = 4.5f;
		m_mGameObjects.emplace(cubeObj.id(), std::move(cubeObj));

		auto pointLight = Game::GameObject::make_point_light(.05f, glm::vec3(0, 1.f, 1.f), 1.f);
		m_mGameObjects.emplace(pointLight.id(), std::move(pointLight));
	}

	// TODO: Make this not a complete mess
	void App::loop(void)
	{
		std::vector<std::unique_ptr<Rendering::Buffer>> vUboBuffers(Rendering::Swapchain::MAX_FRAMES_IN_FLIGHT);
		for (size_t i{}; i < vUboBuffers.size(); i++)
		{
			vUboBuffers[i] = std::make_unique<Rendering::Buffer>(m_device,
																sizeof(Rendering::GlobalUBO),
																1,
																VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
																VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
			vUboBuffers[i]->map();
		}

		auto globalSetLayout = Rendering::DescriptorSetLayout::Builder(m_device)
			.binding_add(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
			.build();

		std::vector<VkDescriptorSet> vGlobalDescriptorSets(Rendering::Swapchain::MAX_FRAMES_IN_FLIGHT);
		for (size_t i{}; i < vGlobalDescriptorSets.size(); i++)
		{
			auto bufferInfo = vUboBuffers[i]->descriptor_info();
			Rendering::DescriptorWriter(*globalSetLayout, *m_pGlobalDescriptorPool)
				.buffer_write(0, &bufferInfo)
				.build(vGlobalDescriptorSets[i]);
		}

		Rendering::RenderSystem renderSystem{
			m_device,
			m_renderer.swapchain_renderpass_get(),
			globalSetLayout->descriptor_set_layout_get()};

		Rendering::BillboardSystem billboardSystem{
			m_device,
			m_renderer.swapchain_renderpass_get(),
			globalSetLayout->descriptor_set_layout_get()};

		Rendering::Camera camera{};

		auto cameraObject = Game::GameObject::create();
		Input::KeyboardMovementController cameraController{};

		// Call this last for accurate timestamps
		auto currentTime = std::chrono::high_resolution_clock::now();
		float totalTime = 0;

		while (!m_window.should_close())
		{
			glfwPollEvents();

			auto newTime = std::chrono::high_resolution_clock::now();
			float frame_dt = std::chrono::duration<float, std::ratio<1>>(newTime - currentTime).count();
			frame_dt = glm::min(frame_dt, ProgramConstants::MAX_FRAME_TIME_SEC);
			currentTime = newTime;
			totalTime += frame_dt;

			cameraController.move_in_plane_xz(m_window.GLFWwindow_get(), cameraObject, frame_dt);
			camera.set_view_yxz(cameraObject.transform.translation, cameraObject.transform.eulerRotationRadians);
			camera.projection_perspective_set(glm::radians(50.f), m_renderer.aspect_ratio(), 0.1f, 50.f);

			// Test "animations"
			float a = 1.5f;
			float w = 1.f;
			glm::vec3 circleCenter(0.f, 2.f, 0.f);
			for (auto &kvp : m_mGameObjects)
			{
				auto &go = kvp.second;
				if (go.m_pPointLight)
				{
					go.transform.translation.x = circleCenter.x + a * glm::sin(w * totalTime);
					go.transform.translation.z = circleCenter.z + a * glm::cos(w * totalTime);
					go.transform.translation.y = circleCenter.y + a * .5f * glm::sin(w * 4.f * totalTime);

					float t = totalTime * 1.5f;

					go.color.r = 0.5f + 0.5f * sin(t);
					go.color.g = 0.5f + 0.5f * sin(t + glm::two_pi<float>() / 3.0f);
					go.color.b = 0.5f + 0.5f * sin(t + 2.0f * glm::two_pi<float>() / 3.0f);
				}
			}
			
			if (auto commandBuffer = m_renderer.frame_begin())
			{
				int frameIndex = m_renderer.frame_index_get();
				Rendering::FrameInfo frameInfo{
					frameIndex,
					frame_dt,
					commandBuffer,
					camera,
					vGlobalDescriptorSets[frameIndex],
					m_mGameObjects};

				// Phase 1: Update
				Rendering::GlobalUBO ubo{};
				ubo.projectionMatrix = camera.projection_get();
				ubo.viewMatrix = camera.view_get();
				billboardSystem.update(frameInfo, ubo);
				
				vUboBuffers[frameIndex]->write_to_buffer(&ubo);
				vUboBuffers[frameIndex]->flush();

				// Phase 2: Draw calls
				m_renderer.swapchain_renderpass_begin(commandBuffer);
				renderSystem.game_objects_render(frameInfo);
				billboardSystem.render(frameInfo);
				m_renderer.swapchain_renderpass_end(commandBuffer);
				m_renderer.frame_end();
			}
		}

		m_renderer.wait_idle();
	}
}