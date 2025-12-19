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

namespace Rendering
{
	Rendering::Rendering(App::App &rApp) : m_rApp{ rApp },
		window{ Window(DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT, APPLICATION_NAME) },
		m_device{ window },
		m_swapchain{ m_device, window.get_extent() }
	{
		create_pipeline_layout();
		create_pipeline();
		create_command_buffers();

		ksc_log::debug("Rendering created.");
	}

	Rendering::~Rendering()
	{
		wait_idle();
		ksc_log::debug("Destroying rendering...");

		vkDestroyPipelineLayout(m_device.device(), m_pipelineLayout, nullptr);
	}

	void Rendering::loop()
	{
		//ksc_log::debug("Render loop");
		draw_frame();
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
		VkPipelineLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		layoutInfo.setLayoutCount = 0;
		layoutInfo.pSetLayouts = nullptr;
		layoutInfo.pushConstantRangeCount = 0;
		layoutInfo.pPushConstantRanges = nullptr;

		if (vkCreatePipelineLayout(m_device.device(), &layoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS)
		{
			const char *pErrorMessage = "Failed to create pipeline layout!";
			ksc_log::error(pErrorMessage);
			throw std::runtime_error(pErrorMessage);
		}
	}

	void Rendering::create_pipeline()
	{
		PipelineConfigInfo pipelineConfig{};
		Pipeline::default_pipeline_config_info(pipelineConfig, m_swapchain.width(), m_swapchain.height());
		pipelineConfig.renderPass = m_swapchain.get_render_pass();
		pipelineConfig.pipelineLayout = m_pipelineLayout;
		m_pPipeline = std::make_unique<Pipeline>(m_device,
												 (m_rApp.get_root() / "res/shaders/simple_shader.vert.spv").generic_string(),
												 (m_rApp.get_root() / "res/shaders/simple_shader.frag.spv").generic_string(),
												 pipelineConfig);
	}

	void Rendering::create_command_buffers()
	{
		m_vCommandBuffers.resize(m_swapchain.image_count());

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

		const size_t COMMAND_BUFFER_COUNT = m_vCommandBuffers.size();
		for (size_t i = 0; i < COMMAND_BUFFER_COUNT; i++)
		{
			VkCommandBufferBeginInfo cmdBeginInfo{};
			cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

			if (vkBeginCommandBuffer(m_vCommandBuffers[i], &cmdBeginInfo) != VK_SUCCESS)
			{
				std::string errorMessage = std::format("Failed to begin recording command buffer {}/{}!", i, COMMAND_BUFFER_COUNT);
				ksc_log::error(errorMessage);
				throw std::runtime_error(errorMessage);
			}

			VkRenderPassBeginInfo rendBeginInfo{};
			rendBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			rendBeginInfo.renderPass = m_swapchain.get_render_pass();
			rendBeginInfo.framebuffer = m_swapchain.get_framebuffer(i);

			rendBeginInfo.renderArea.offset = { 0, 0 };
			rendBeginInfo.renderArea.extent = m_swapchain.get_swapchain_extent();

			std::array<VkClearValue, 2> aClearValues{};
			aClearValues[0].color = { 0.1f, 0.1f, 0.1f, 1.0f };
			aClearValues[1].depthStencil = { 1.0f, 0 };

			rendBeginInfo.clearValueCount = static_cast<uint32_t>(aClearValues.size());
			rendBeginInfo.pClearValues = aClearValues.data();

			vkCmdBeginRenderPass(m_vCommandBuffers[i], &rendBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

			m_pPipeline->bind(m_vCommandBuffers[i]);
			vkCmdDraw(m_vCommandBuffers[i], 3, 1, 0, 0);

			vkCmdEndRenderPass(m_vCommandBuffers[i]);
			if (vkEndCommandBuffer(m_vCommandBuffers[i]) != VK_SUCCESS)
			{
				std::string errorMessage = std::format("Failed to record command buffer {}/{}!", i, COMMAND_BUFFER_COUNT);
				ksc_log::error(errorMessage);
				throw std::runtime_error(errorMessage);
			}
		}
	}

	void Rendering::draw_frame()
	{
		uint32_t imageIndex;
		auto result = m_swapchain.acquire_next_image(&imageIndex);

		// TODO: Don't do this once window resize is enabled.
		if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
		{
			std::string errorMessage = std::format("Failed to aquire swapchain image {}/{}! You forgot to change window resizing behavior.",
												   imageIndex, m_swapchain.image_count());
			ksc_log::error(errorMessage);
			throw std::runtime_error(errorMessage);
		}

		result = m_swapchain.submit_command_buffer(&m_vCommandBuffers[imageIndex], &imageIndex);
		if (result != VK_SUCCESS)
		{
			std::string errorMessage = std::format("Failed to present swapchain image {}/{}!", imageIndex, m_swapchain.image_count());
			ksc_log::error(errorMessage);
			throw std::runtime_error(errorMessage);
		}
	}
}