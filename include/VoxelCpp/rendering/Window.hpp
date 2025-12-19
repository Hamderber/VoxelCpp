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

		bool poll_or_exit();
		void create_surface(VkInstance instance, VkSurfaceKHR *pSurface);
		VkExtent2D get_extent() { return { static_cast<uint32_t>(WIDTH), static_cast<uint32_t>(HEIGHT) }; };

		const int WIDTH;
		const int HEIGHT;

	private:
		const char *m_pNAME;
		GLFWwindow *m_pWindow;
	};
}