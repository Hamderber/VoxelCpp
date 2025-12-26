#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan_core.h>
#include <cstdint>

namespace Rendering
{
	class Window
	{
	public:
		Window(int w, int h, const char *pNAME);
		~Window();

		Window(const Window &) = delete;
		Window &operator=(const Window &) = delete;

		GLFWwindow *GLFWwindow_get() const { return m_pWindow; }

		bool should_close();
		void create_surface(VkInstance instance, VkSurfaceKHR *pSurface);
		VkExtent2D get_extent() { return { static_cast<uint32_t>(m_width), static_cast<uint32_t>(m_height) }; }
		bool flag_get_was_window_resized() const { return m_framebufferResized; }
		void flag_reset_was_window_resized() { m_framebufferResized = false; }

	private:
		static void framebuffer_resize_callback(GLFWwindow *pGLFWwindow, int width, int height);

		int m_width;
		int m_height;
		const char *m_pNAME;
		GLFWwindow *m_pWindow;

		// Flags
		bool m_framebufferResized = false;
	};
}