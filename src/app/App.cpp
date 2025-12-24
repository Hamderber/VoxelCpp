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

namespace App
{
	App::App(std::filesystem::path root) : m_root{root}
	{
		game_objects_load();
		pipeline_layout_create();
		pipeline_create();

		ksc_log::debug("App created. Root: " + m_root.generic_string());
	}

	App::~App()
	{
		ksc_log::debug("Destroying app...");
		pipeline_layout_destroy();
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

	void App::pipeline_layout_create()
	{
		VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(Rendering::SimplePushConstantData);

		VkPipelineLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		layoutInfo.setLayoutCount = 0;
		layoutInfo.pSetLayouts = nullptr;
		layoutInfo.pushConstantRangeCount = 1;
		layoutInfo.pPushConstantRanges = &pushConstantRange;

		if (vkCreatePipelineLayout(m_device.device(), &layoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS)
		{
			const char *pErrorMessage = "Failed to create pipeline layout!";
			ksc_log::error(pErrorMessage);
			throw std::runtime_error(pErrorMessage);
		}
	}

	void App::pipeline_layout_destroy()
	{
		vkDestroyPipelineLayout(m_device.device(), m_pipelineLayout, nullptr);
	}

	void App::pipeline_create()
	{
		assert(m_pipelineLayout && "Cannot create the pipeline before the pipeline layout!");

		Rendering::PipelineConfigInfo pipelineConfig{};
		Rendering::Pipeline::default_pipeline_config_info(pipelineConfig);
		pipelineConfig.renderPass = m_renderer.swapchain_renderpass_get();
		pipelineConfig.pipelineLayout = m_pipelineLayout;
		m_pPipeline = std::make_unique<Rendering::Pipeline>(m_device,
												 (m_root / "res/shaders/simple_shader.vert.spv").generic_string(),
												 (m_root / "res/shaders/simple_shader.frag.spv").generic_string(),
												 pipelineConfig);
	}

	void App::game_objects_render(VkCommandBuffer commandBuffer)
	{
		m_pPipeline->bind(commandBuffer);

		for (auto &go : m_vGameObjects)
		{
			// TODO: something like time.deltaTime
			go.transform2D.rotationRadians = glm::mod(go.transform2D.rotationRadians + 0.0001f, glm::two_pi<float>());

			Rendering::SimplePushConstantData push{};
			push.offset = go.transform2D.translation;
			push.color = go.color;
			push.transform = go.transform2D.matrix();

			vkCmdPushConstants((commandBuffer), m_pipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT,
							   0, static_cast<uint32_t>(sizeof(Rendering::SimplePushConstantData)), &push);

			go.pModel->bind(commandBuffer);
			go.pModel->draw(commandBuffer);
		}
	}

	void App::loop(void)
	{
		while (!m_window.should_close())
		{
			glfwPollEvents();

			if (auto commandBuffer = m_renderer.frame_begin())
			{
				m_renderer.swapchain_renderpass_begin(commandBuffer);
				game_objects_render(commandBuffer);
				m_renderer.swapchain_renderpass_end(commandBuffer);
				m_renderer.frame_end();
			}
		}

		m_renderer.wait_idle();
	}
}