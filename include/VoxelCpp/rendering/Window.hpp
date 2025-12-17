#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan_core.h>

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
		
		const int WIDTH;
		const int HEIGHT;

	private:
		const char *m_pNAME;
		GLFWwindow *m_pWindow;
	};
}