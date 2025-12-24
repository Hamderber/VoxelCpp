#pragma once
#include <vulkan/vulkan_core.h>
#include <vector>
#include <cstdint>
#include <memory>

namespace Rendering { class Device; };

namespace Rendering
{
    class Swapchain
    {
    public:
        static constexpr size_t MAX_FRAMES_IN_FLIGHT = 2;

        Swapchain(Device &rDevice, VkExtent2D windowExtent);
        Swapchain(Device &rDevice, VkExtent2D windowExtent, std::shared_ptr<Swapchain> pPrevious);
        ~Swapchain();

        Swapchain(const Swapchain &) = delete;
        Swapchain operator=(const Swapchain &) = delete;

        VkFramebuffer get_framebuffer(int index) { return m_vFramebuffers[index]; }
        VkRenderPass get_render_pass() const { return m_renderPass; }
        VkImageView get_image_view(int index) { return m_vImageViews[index]; }
        size_t image_count() { return m_vImages.size(); }
        VkFormat get_swapchain_image_format() const { return m_imageFormat; }
        VkExtent2D get_extent() const { return m_extent; }
        uint32_t width() const { return m_extent.width; }
        uint32_t height() const { return m_extent.height; }

        float extent_aspect_ratio() const
        {
            return static_cast<float>(m_extent.width) / static_cast<float>(m_extent.height);
        }

        VkFormat find_depth_format();

        VkResult acquire_next_image(uint32_t *pImageIndex);
        VkResult submit_command_buffer(const VkCommandBuffer *pBUFFER, uint32_t *pImageIndex);

        bool compare_swap_formats(const Swapchain &rSWAPCHAIN) const
        {
            return rSWAPCHAIN.m_depthFormat == m_depthFormat && rSWAPCHAIN.m_imageFormat == m_imageFormat;
        }

    private:
        void init();
        void create_swapchain();
        void create_image_views();
        void create_depth_resources();
        void create_render_pass();
        void create_framebuffers();
        void create_sync_objects();

        VkSurfaceFormatKHR choose_swapchain_surface_format(const std::vector<VkSurfaceFormatKHR> &rAVAILABLE_FORMATS);
        VkPresentModeKHR choose_swapchain_present_mode(const std::vector<VkPresentModeKHR> &rAVAILABLE_PRESENT_MODES);
        VkExtent2D choose_swapchain_extent(const VkSurfaceCapabilitiesKHR &rCAPABILITIES) const;

        VkFormat m_imageFormat;
        VkFormat m_depthFormat;
        VkExtent2D m_extent;

        std::vector<VkFramebuffer> m_vFramebuffers;
        VkRenderPass m_renderPass;

        std::vector<VkImage> m_vDepthImages;
        std::vector<VkDeviceMemory> m_vDepthImageMemorys;
        std::vector<VkImageView> m_vDepthImageViews;
        std::vector<VkImage> m_vImages;
        std::vector<VkImageView> m_vImageViews;

        Device &m_rDevice;
        VkExtent2D m_windowExtent;

        VkSwapchainKHR m_swapchain;
        std::shared_ptr<Swapchain> pOldSwapchain;

        std::vector<VkSemaphore> m_vImageAvailableSemaphores;
        std::vector<VkSemaphore> m_vRenderFinishedSemaphores;
        std::vector<VkFence> m_vInFlightFences;
        std::vector<VkFence> m_vImagesInFlight;
        size_t m_currentFrame = 0;
    };
}