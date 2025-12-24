#include <ksc_log.hpp>
#include <VoxelCpp/rendering/Renderer.hpp>
#include <VoxelCpp/rendering/Window.hpp>
#include <stdexcept>
#include <vulkan/vulkan_core.h>
#include <memory>
#include <cstdint>
#include <format>
#include <string>
#include <array>
#include <vector>
#include <VoxelCpp/rendering/Swapchain.hpp>
#include <GLFW/glfw3.h>
#include <utility>
#include <cassert>
#include <VoxelCpp/rendering/Device.hpp>

namespace Rendering
{
	Renderer::Renderer(Window &rWindow, Device &rDevice) : m_rWindow{ rWindow }, m_rDevice{ rDevice }
	{
		ksc_log::debug("Creating Renderer...");
		swapchain_recreate();
		command_buffers_create();
	}

	Renderer::~Renderer()
	{
		ksc_log::debug("Destroying Renderer...");
		wait_idle();

		command_buffers_destroy();
	}

	VkCommandBuffer Renderer::frame_begin()
	{
		assert(!m_isFrameStarted && "Can't begin a frame while one is already in progress.");
		auto result = m_pSwapchain->acquire_next_image(&m_currentImageIndex);

		if (result == VK_ERROR_OUT_OF_DATE_KHR)
		{
			swapchain_recreate();
			return nullptr;
		}

		if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
		{
			std::string errorMessage = std::format("Failed to aquire swapchain image {}/{}! You forgot to change window resizing behavior.",
												   m_currentImageIndex, m_pSwapchain->image_count());
			ksc_log::error(errorMessage);
			throw std::runtime_error(errorMessage);
		}

		m_isFrameStarted = true;
		auto commandBuffer = command_buffer_get_current();
		VkCommandBufferBeginInfo cmdBeginInfo{};
		cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		if (vkBeginCommandBuffer(commandBuffer, &cmdBeginInfo) != VK_SUCCESS)
		{
			std::string errorMessage = std::format("Failed to begin recording command buffer {}/{}!",
												   m_currentImageIndex, m_vCommandBuffers.size());
			ksc_log::error(errorMessage);
			throw std::runtime_error(errorMessage);
		}

		return commandBuffer;
	}

	void Renderer::frame_end()
	{
		assert(m_isFrameStarted && "Can't end a frame when one isn't in progress.");
		auto commandBuffer = command_buffer_get_current();

		if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
		{
			std::string errorMessage = std::format("Failed to record command buffer {}/{}!",
												   m_currentImageIndex, m_vCommandBuffers.size());
			ksc_log::error(errorMessage);
			throw std::runtime_error(errorMessage);
		}

		auto result = m_pSwapchain->submit_command_buffer(&commandBuffer, &m_currentImageIndex);
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_rWindow.flag_get_was_window_resized())
		{
			m_rWindow.flag_reset_was_window_resized();
			swapchain_recreate();
		}
		else if (result != VK_SUCCESS)
		{
			std::string errorMessage = std::format("Failed to present swapchain image {}/{}!",
												   m_currentImageIndex, m_pSwapchain->image_count());
			ksc_log::error(errorMessage);
			throw std::runtime_error(errorMessage);
		}

		m_isFrameStarted = false;
	}

	void Renderer::swapchain_renderpass_begin(VkCommandBuffer commandBuffer)
	{
		assert(m_isFrameStarted && "Can't begin swapchain renderpass when there isn't a frame in progress.");
		assert(commandBuffer == command_buffer_get_current() &&
			   "Can't begin swapchain renderpass with a command buffer from a different frame.");

		VkRenderPassBeginInfo rendBeginInfo{};
		rendBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		rendBeginInfo.renderPass = m_pSwapchain->get_render_pass();
		rendBeginInfo.framebuffer = m_pSwapchain->get_framebuffer(m_currentImageIndex);

		rendBeginInfo.renderArea.offset = { 0, 0 };
		rendBeginInfo.renderArea.extent = m_pSwapchain->get_extent();

		std::array<VkClearValue, 2> aClearValues{};
		aClearValues[0].color = { 0.1f, 0.1f, 0.1f, 1.0f };
		aClearValues[1].depthStencil = { 1.0f, 0 };

		rendBeginInfo.clearValueCount = static_cast<uint32_t>(aClearValues.size());
		rendBeginInfo.pClearValues = aClearValues.data();

		vkCmdBeginRenderPass(commandBuffer, &rendBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

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

		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
	}

	void Renderer::swapchain_renderpass_end(VkCommandBuffer commandBuffer) const
	{
		assert(m_isFrameStarted && "Can't end swapchain renderpass when there isn't a frame in progress.");
		assert(commandBuffer == command_buffer_get_current() &&
			   "Can't end swapchain renderpass with a command buffer from a different frame.");

		vkCmdEndRenderPass(commandBuffer);
	}

	void Renderer::wait_idle()
	{
		if (vkDeviceWaitIdle(m_rDevice.device()) != VK_SUCCESS)
		{
			const char *pErrorMessage = "Failed to wait for the device to be idle!";
			ksc_log::error(pErrorMessage);
			throw std::runtime_error(pErrorMessage);
		}
	}

	void Renderer::command_buffers_create()
	{
		m_vCommandBuffers.resize(m_pSwapchain->image_count());

		VkCommandBufferAllocateInfo allocateInfo{};
		allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocateInfo.commandPool = m_rDevice.get_command_pool();
		allocateInfo.commandBufferCount = static_cast<uint32_t>(m_vCommandBuffers.size());
	
		if (vkAllocateCommandBuffers(m_rDevice.device(), &allocateInfo, m_vCommandBuffers.data()) != VK_SUCCESS)
		{
			const char *pErrorMessage = "Failed to allocate command buffers!";
			ksc_log::error(pErrorMessage);
			throw std::runtime_error(pErrorMessage);
		}
	}

	void Renderer::command_buffers_destroy()
	{
		vkFreeCommandBuffers(m_rDevice.device(), m_rDevice.get_command_pool(), static_cast<uint32_t>(m_vCommandBuffers.size()),
							 m_vCommandBuffers.data());
		m_vCommandBuffers.clear();
	}

	void Renderer::swapchain_recreate()
	{
		auto extent = m_rWindow.get_extent();
		while (extent.width == 0 || extent.height == 0)
		{
			extent = m_rWindow.get_extent();
			glfwWaitEvents();
		}

		vkDeviceWaitIdle(m_rDevice.device());

		if (!m_pSwapchain)
		{
			m_pSwapchain = std::make_unique<Swapchain>(m_rDevice, m_rWindow.get_extent());
		}
		else
		{
			m_pSwapchain = std::make_unique<Swapchain>(m_rDevice, m_rWindow.get_extent(), std::move(m_pSwapchain));
			if (m_pSwapchain->image_count() != m_vCommandBuffers.size())
			{
				command_buffers_destroy();
				command_buffers_create();
			}
		}
	}
}