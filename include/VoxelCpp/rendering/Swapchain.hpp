#pragma once
#include <vulkan/vulkan_core.h>
#include <vector>
#include <cstdint>

namespace Rendering { class Device; };

namespace Rendering
{
    class Swapchain
    {
    public:
        static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

        Swapchain(Device &rDevice, VkExtent2D windowExtent);
        ~Swapchain();

        Swapchain(const Swapchain &) = delete;
        void operator=(const Swapchain &) = delete;

        VkFramebuffer get_framebuffer(int index) { return m_vSwapchainFramebuffers[index]; }
        VkRenderPass get_render_pass() const { return m_renderPass; }
        VkImageView get_image_view(int index) { return m_vSwapchainImageViews[index]; }
        size_t image_count() { return m_vSwapchainImages.size(); }
        VkFormat get_swapchain_image_format() const { return m_swapchainImageFormat; }
        VkExtent2D get_swapchain_extent() const { return m_swapchainExtent; }
        uint32_t width() const { return m_swapchainExtent.width; }
        uint32_t height() const { return m_swapchainExtent.height; }

        float extent_aspect_ratio() const
        {
            return static_cast<float>(m_swapchainExtent.width) / static_cast<float>(m_swapchainExtent.height);
        }

        VkFormat find_depth_format();

        VkResult acquire_next_image(uint32_t *pImageIndex);
        VkResult submit_command_buffer(const VkCommandBuffer *pBUFFER, uint32_t *pImageIndex);

    private:
        void create_swapchain();
        void create_image_views();
        void create_depth_resources();
        void create_render_pass();
        void create_framebuffers();
        void create_sync_objects();

        VkSurfaceFormatKHR choose_swapchain_surface_format(const std::vector<VkSurfaceFormatKHR> &rAVAILABLE_FORMATS);
        VkPresentModeKHR choose_swapchain_present_mode(const std::vector<VkPresentModeKHR> &rAVAILABLE_PRESENT_MODES);
        VkExtent2D choose_swapchain_extent(const VkSurfaceCapabilitiesKHR &rCAPABILITIES) const;

        VkFormat m_swapchainImageFormat;
        VkExtent2D m_swapchainExtent;

        std::vector<VkFramebuffer> m_vSwapchainFramebuffers;
        VkRenderPass m_renderPass;

        std::vector<VkImage> m_vDepthImages;
        std::vector<VkDeviceMemory> m_vDepthImageMemorys;
        std::vector<VkImageView> m_vDepthImageViews;
        std::vector<VkImage> m_vSwapchainImages;
        std::vector<VkImageView> m_vSwapchainImageViews;

        Device &m_rDevice;
        VkExtent2D m_windowExtent;

        VkSwapchainKHR m_swapchain;

        std::vector<VkSemaphore> m_vImageAvailableSemaphores;
        std::vector<VkSemaphore> m_vRenderFinishedSemaphores;
        std::vector<VkFence> m_vInFlightFences;
        std::vector<VkFence> m_vImagesInFlight;
        size_t m_currentFrame = 0;
    };
}