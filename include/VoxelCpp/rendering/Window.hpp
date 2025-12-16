#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

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
	private:
		const int m_WIDTH;
		const int m_HEIGHT;
		const char *m_pNAME;
		GLFWwindow *m_pWindow;
	};
}