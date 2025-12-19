#pragma once
#include <vector>
#include <vulkan/vulkan_core.h>
#include <cstdint>

namespace Rendering { class Window; };

namespace Rendering
{
	struct SwapchainSupportDetails
	{
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> vFormats;
		std::vector<VkPresentModeKHR> vPresentModes;
	};

	struct QueueFamilyIndices
	{
		uint32_t graphicsFamily = 0;
		uint32_t presentFamily = 0;
		bool graphicsFamilyHasValue = false;
		bool presentFamilyHasValue = false;

		bool is_complete() const { return graphicsFamilyHasValue && presentFamilyHasValue; };
	};

	class Device
	{
	public:
#ifdef NDEBUG
		const bool ENABLE_VALIDATION_LAYERS = false;
#else
		const bool ENABLE_VALIDATION_LAYERS = true;
#endif
		Device(Window &rWindow);
		~Device();

		Device(const Device &) = delete;
		void operator=(const Device &) = delete;
		Device(Device &&) = delete;
		Device &operator=(Device &&) = delete;

		VkCommandPool get_command_pool() const { return m_commandPool; }
		VkDevice device() const { return m_device; }
		VkSurfaceKHR surface() const { return m_surface; }
		VkQueue graphics_queue() const { return m_graphicsQueue; }
		VkQueue present_queue() const { return m_presentQueue; }

		SwapchainSupportDetails get_swapchain_support() { return query_swapchain_support(m_physicalDevice); }
		uint32_t find_memory_type(uint32_t typeFilter, VkMemoryPropertyFlags properties) const;
		QueueFamilyIndices find_physical_queue_families() { return find_queue_families(m_physicalDevice); }

		VkFormat find_supported_format(const std::vector<VkFormat> &rCANDIDATES, VkImageTiling tiling, VkFormatFeatureFlags features) const;
		void create_buffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer &rBuffer,
						  VkDeviceMemory &rBufferMemory) const;
		VkCommandBuffer begin_single_time_commands() const;

		void end_single_time_commands(VkCommandBuffer commandBuffer) const;
		void copy_buffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) const;
		void copy_buffer_to_image(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t layerCount) const;
		void create_image_with_info(const VkImageCreateInfo &rIMAGE_INFO, VkMemoryPropertyFlags properties, VkImage &image,
									VkDeviceMemory &rImageMemory) const;

		VkPhysicalDeviceProperties properties;

	private:
		void create_instance();
		void setup_debug_messenger();
		void create_surface();
		void pick_physical_device();
		void create_logical_device();
		void create_command_pool();
		bool is_device_suitable(VkPhysicalDevice device);
		std::vector<const char *> get_required_extensions() const;
		bool check_validation_layer_support();
		QueueFamilyIndices find_queue_families(VkPhysicalDevice device) const;
		void populate_debug_messenger_create_info(VkDebugUtilsMessengerCreateInfoEXT &rCreateInfo);
		void has_gflw_required_instance_extensions();
		bool check_device_extension_support(VkPhysicalDevice device);
		SwapchainSupportDetails query_swapchain_support(VkPhysicalDevice device) const;

		VkInstance m_instance;
		VkDebugUtilsMessengerEXT m_debugMessenger;
		VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
		Window &m_rWindow;
		VkCommandPool m_commandPool;

		VkDevice m_device;
		VkSurfaceKHR m_surface;
		VkQueue m_graphicsQueue;
		VkQueue m_presentQueue;

		const std::vector<const char *> m_vValidationLayers = { "VK_LAYER_KHRONOS_validation" };
		const std::vector<const char *> m_vDeviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
	};
}