#include <VoxelCpp/rendering/Window.hpp>
#include <cstdint>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan_core.h>
#include <ksc_log.hpp>
#include <format>
#include <stdexcept>

namespace Rendering
{
	Window::Window(int w, int h, const char *pNAME) : m_width{ w }, m_height{ h }, m_pNAME{ pNAME }, m_pWindow{ nullptr }
	{
		if (!glfwInit())
		{
			ksc_log::error("glfwInit failed!");
			return;
		}

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

		m_pWindow = glfwCreateWindow(m_width, m_height, m_pNAME, nullptr, nullptr);
		if (!m_pWindow)
		{
			ksc_log::debug(std::format("glfwCreateWindow failed: {}", glfwGetError(nullptr)));
			return;
		}

		glfwSetWindowUserPointer(m_pWindow, this);
		glfwSetFramebufferSizeCallback(m_pWindow, framebuffer_resize_callback);

		uint32_t extensionCount = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
		ksc_log::debug(std::format("{} Vulkan extensions are supported.", extensionCount));

		ksc_log::debug("Window created.");
	}

	Window::~Window()
	{
		ksc_log::debug("Destroying window...");

		glfwDestroyWindow(m_pWindow);
		glfwTerminate();
	}

	bool Window::poll_or_exit()
	{
		glfwPollEvents();

		return glfwWindowShouldClose(m_pWindow);
	}

	void Window::create_surface(VkInstance instance, VkSurfaceKHR *pSurface)
	{
		if (glfwCreateWindowSurface(instance, m_pWindow, nullptr, pSurface) != VK_SUCCESS)
		{
			const char *pErrorMessage = "Failed to create window surface!";
			ksc_log::error(pErrorMessage);
			throw std::runtime_error(pErrorMessage);
		}
	}

	void Window::framebuffer_resize_callback(GLFWwindow *pGLFWwindow, int width, int height)
	{
		auto pWindow = reinterpret_cast<Window *>(glfwGetWindowUserPointer(pGLFWwindow));
		pWindow->m_framebufferResized = true;
		pWindow->m_width = width;
		pWindow->m_height = height;
	}
}