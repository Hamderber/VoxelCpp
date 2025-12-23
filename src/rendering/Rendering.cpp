#include <VoxelCpp/app/ProgramConstants.hpp>
#include <VoxelCpp/app/App.hpp>
#include <ksc_log.hpp>
#include <VoxelCpp/rendering/Rendering.hpp>
#include <VoxelCpp/rendering/Window.hpp>
#include <VoxelCpp/rendering/Pipeline.hpp>
#include <stdexcept>
#include <vulkan/vulkan_core.h>
#include <memory>
#include <cstdint>
#include <format>
#include <string>
#include <array>
#include <VoxelCpp/rendering/Model.hpp>
#include <vector>
#include <VoxelCpp/rendering/Swapchain.hpp>
#include <GLFW/glfw3.h>
#include <utility>
#include <cassert>
#include <VoxelCpp/game/GameObject.hpp>
#include <glm/gtc/constants.hpp>

namespace Rendering
{
	Rendering::Rendering(App::App &rApp) : m_rApp{ rApp },
		window{ Window(DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT, APPLICATION_NAME) },
		m_device{ window }
	{
		load_game_objects();
		create_pipeline_layout();
		recreate_swapchain();
		create_command_buffers();

		ksc_log::debug("Rendering created.");
	}

	Rendering::~Rendering()
	{
		wait_idle();
		ksc_log::debug("Destroying rendering...");

		vkDestroyPipelineLayout(m_device.device(), m_pPipelineLayout, nullptr);
	}

	void Rendering::loop()
	{
		//ksc_log::debug("Render loop");
		draw_frame();
	}

	void Rendering::load_game_objects()
	{
		// Hello world triangle (model version)
		std::vector<Model::Vertex> vVerticies {
			{{ 0.0f, -0.5f }, {1.0f, 0, 0}},
			{{ 0.5f, 0.5f }, {0, 1.0f, 0}},
			{{ -0.5f, 0.5f }, {0, 0, 1.0f}}
		};

		auto pModel = std::make_shared<Model>(m_device, vVerticies);

		auto triangleGO = Game::GameObject::create();
		triangleGO.pModel = pModel;
		triangleGO.color = { 0.1f, 0.8f, 0.1f };
		triangleGO.transform2D.translation.x = 0.2f;
		triangleGO.transform2D.scale = { 2, 0.5f };
		triangleGO.transform2D.rotationRadians = 0.0025f * glm::two_pi<float>();

		m_vGameObjects.push_back(std::move(triangleGO));
	}

	void Rendering::wait_idle()
	{
		if (vkDeviceWaitIdle(m_device.device()) != VK_SUCCESS)
		{
			const char *pErrorMessage = "Failed to wait for the device to be idle!";
			ksc_log::error(pErrorMessage);
			throw std::runtime_error(pErrorMessage);
		}
	}

	void Rendering::create_pipeline_layout()
	{
		VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(SimplePushConstantData);

		VkPipelineLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		layoutInfo.setLayoutCount = 0;
		layoutInfo.pSetLayouts = nullptr;
		layoutInfo.pushConstantRangeCount = 1;
		layoutInfo.pPushConstantRanges = &pushConstantRange;

		if (vkCreatePipelineLayout(m_device.device(), &layoutInfo, nullptr, &m_pPipelineLayout) != VK_SUCCESS)
		{
			const char *pErrorMessage = "Failed to create pipeline layout!";
			ksc_log::error(pErrorMessage);
			throw std::runtime_error(pErrorMessage);
		}
	}

	void Rendering::create_pipeline()
	{
		assert(m_pSwapchain && "Cannot create the pipeline before the swapchain!");
		assert(m_pPipelineLayout && "Cannot create the pipeline before the pipeline layout!");

		PipelineConfigInfo pipelineConfig{};
		Pipeline::default_pipeline_config_info(pipelineConfig);
		pipelineConfig.renderPass = m_pSwapchain->get_render_pass();
		pipelineConfig.pipelineLayout = m_pPipelineLayout;
		m_pPipeline = std::make_unique<Pipeline>(m_device,
												 (m_rApp.get_root() / "res/shaders/simple_shader.vert.spv").generic_string(),
												 (m_rApp.get_root() / "res/shaders/simple_shader.frag.spv").generic_string(),
												 pipelineConfig);
	}

	void Rendering::create_command_buffers()
	{
		m_vCommandBuffers.resize(m_pSwapchain->image_count());

		VkCommandBufferAllocateInfo allocateInfo{};
		allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocateInfo.commandPool = m_device.get_command_pool();
		allocateInfo.commandBufferCount = static_cast<uint32_t>(m_vCommandBuffers.size());
	
		if (vkAllocateCommandBuffers(m_device.device(), &allocateInfo, m_vCommandBuffers.data()) != VK_SUCCESS)
		{
			const char *pErrorMessage = "Failed to allocate command buffers!";
			ksc_log::error(pErrorMessage);
			throw std::runtime_error(pErrorMessage);
		}
	}

	void Rendering::destroy_command_buffers()
	{
		vkFreeCommandBuffers(m_device.device(), m_device.get_command_pool(), static_cast<uint32_t>(m_vCommandBuffers.size()),
							 m_vCommandBuffers.data());
		m_vCommandBuffers.clear();
	}

	void Rendering::draw_frame()
	{
		uint32_t imageIndex;
		auto result = m_pSwapchain->acquire_next_image(&imageIndex);

		if (result == VK_ERROR_OUT_OF_DATE_KHR)
		{
			recreate_swapchain();
			return;
		}

		if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
		{
			std::string errorMessage = std::format("Failed to aquire swapchain image {}/{}! You forgot to change window resizing behavior.",
												   imageIndex, m_pSwapchain->image_count());
			ksc_log::error(errorMessage);
			throw std::runtime_error(errorMessage);
		}

		record_command_buffer(imageIndex);
		result = m_pSwapchain->submit_command_buffer(&m_vCommandBuffers[imageIndex], &imageIndex);
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || window.flag_get_was_window_resized())
		{
			window.flag_reset_was_window_resized();
			recreate_swapchain();
			return;
		}

		if (result != VK_SUCCESS)
		{
			std::string errorMessage = std::format("Failed to present swapchain image {}/{}!", imageIndex, m_pSwapchain->image_count());
			ksc_log::error(errorMessage);
			throw std::runtime_error(errorMessage);
		}
	}

	void Rendering::recreate_swapchain()
	{
		auto extent = window.get_extent();
		while (extent.width == 0 || extent.height == 0)
		{
			extent = window.get_extent();
			glfwWaitEvents();
		}

		vkDeviceWaitIdle(m_device.device());

		if (!m_pSwapchain)
		{
			m_pSwapchain = std::make_unique<Swapchain>(m_device, window.get_extent());
		}
		else
		{
			m_pSwapchain = std::make_unique<Swapchain>(m_device, window.get_extent(), std::move(m_pSwapchain));
			if (m_pSwapchain->image_count() != m_vCommandBuffers.size())
			{
				destroy_command_buffers();
				create_command_buffers();
			}
		}

		create_pipeline();
	}

	void Rendering::record_command_buffer(uint32_t imageIndex)
	{
		const size_t COMMAND_BUFFER_COUNT = m_vCommandBuffers.size();

		VkCommandBufferBeginInfo cmdBeginInfo{};
		cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		if (vkBeginCommandBuffer(m_vCommandBuffers[imageIndex], &cmdBeginInfo) != VK_SUCCESS)
		{
			std::string errorMessage = std::format("Failed to begin recording command buffer {}/{}!", imageIndex, COMMAND_BUFFER_COUNT);
			ksc_log::error(errorMessage);
			throw std::runtime_error(errorMessage);
		}

		VkRenderPassBeginInfo rendBeginInfo{};
		rendBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		rendBeginInfo.renderPass = m_pSwapchain->get_render_pass();
		rendBeginInfo.framebuffer = m_pSwapchain->get_framebuffer(imageIndex);

		rendBeginInfo.renderArea.offset = { 0, 0 };
		rendBeginInfo.renderArea.extent = m_pSwapchain->get_extent();

		std::array<VkClearValue, 2> aClearValues{};
		aClearValues[0].color = { 0.1f, 0.1f, 0.1f, 1.0f };
		aClearValues[1].depthStencil = { 1.0f, 0 };

		rendBeginInfo.clearValueCount = static_cast<uint32_t>(aClearValues.size());
		rendBeginInfo.pClearValues = aClearValues.data();

		vkCmdBeginRenderPass(m_vCommandBuffers[imageIndex], &rendBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport{};
		viewport.x = 0;
		viewport.y = 0;
		viewport.width = static_cast<float>(m_pSwapchain->get_extent().width);
		viewport.height = static_cast<float>(m_pSwapchain->get_extent().height);
		viewport.minDepth = 0;
		viewport.maxDepth = 1.0f;
		VkRect2D scissor{};
		scissor.offset = { 0,0 };
		scissor.extent = m_pSwapchain->get_extent();

		vkCmdSetViewport(m_vCommandBuffers[imageIndex], 0, 1, &viewport);
		vkCmdSetScissor(m_vCommandBuffers[imageIndex], 0, 1, &scissor);

		render_game_objects(m_vCommandBuffers[imageIndex]);

		vkCmdEndRenderPass(m_vCommandBuffers[imageIndex]);
		if (vkEndCommandBuffer(m_vCommandBuffers[imageIndex]) != VK_SUCCESS)
		{
			std::string errorMessage = std::format("Failed to record command buffer {}/{}!", imageIndex, COMMAND_BUFFER_COUNT);
			ksc_log::error(errorMessage);
			throw std::runtime_error(errorMessage);
		}
	}

	void Rendering::render_game_objects(VkCommandBuffer commandBuffer)
	{
		m_pPipeline->bind(commandBuffer);

		for (auto &go : m_vGameObjects)
		{
			// TODO: something like time.deltaTime
			go.transform2D.rotationRadians = glm::mod(go.transform2D.rotationRadians + 0.0001f, glm::two_pi<float>());

			SimplePushConstantData push{};
			push.offset = go.transform2D.translation;
			push.color = go.color;
			push.transform = go.transform2D.matrix();

			vkCmdPushConstants((commandBuffer), m_pPipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT,
							   0, static_cast<uint32_t>(sizeof(SimplePushConstantData)), &push);
		
			go.pModel->bind(commandBuffer);
			go.pModel->draw(commandBuffer);
		}
	}
}