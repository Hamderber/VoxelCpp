#pragma once
#include <VoxelCpp/rendering/Device.hpp>
#include <VoxelCpp/rendering/Window.hpp>
#include <VoxelCpp/rendering/Swapchain.hpp>
#include <memory>
#include <vulkan/vulkan_core.h>
#include <vector>
#include <cstdint>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/fwd.hpp>
#include <cassert>

namespace Rendering
{
	class Renderer
	{
	public:
		Renderer(Window &rWindow, Device &rDevice);
		~Renderer();

		Renderer(const Renderer &) = delete;
		Renderer &operator=(const Renderer &) = delete;

		const bool frame_is_started() const { return m_isFrameStarted; }
		
		VkCommandBuffer command_buffer_get_current() const
		{ 
			assert(m_isFrameStarted && "Can't get the current command buffer when a frame isn't in progress.");
			return m_vCommandBuffers[m_currentFrameIndex];
		}

		VkRenderPass swapchain_renderpass_get() const { return m_pSwapchain->get_render_pass(); }

		int frame_index_get() const
		{
			assert(m_isFrameStarted && "Can't get the current frame index when a frame isn't in progress.");
			return m_currentFrameIndex;
		}

		VkCommandBuffer frame_begin();
		void frame_end();
		void swapchain_renderpass_begin(VkCommandBuffer commandBuffer);
		void swapchain_renderpass_end(VkCommandBuffer commandBuffer) const;

		void wait_idle();

	private:
		void command_buffers_create();
		void command_buffers_destroy();
		void swapchain_recreate();
		
		Window &m_rWindow;
		Device &m_rDevice;
		std::unique_ptr<Swapchain> m_pSwapchain;
		std::vector<VkCommandBuffer> m_vCommandBuffers;
		uint32_t m_currentImageIndex = 0;
		size_t m_currentFrameIndex = 0;
		bool m_isFrameStarted = false;
	};
}