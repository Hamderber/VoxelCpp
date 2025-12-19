#include <ksc_log.hpp>
#include <VoxelCpp/rendering/Swapchain.hpp>
#include <VoxelCpp/rendering/Device.hpp>
#include <array>
#include <limits>
#include <stdexcept>
#include <algorithm>
#include <cstdint>
#include <vector>
#include <vulkan/vulkan_core.h>
#include <format>
#include <string>

namespace Rendering
{
    Swapchain::Swapchain(Device &rDevice, VkExtent2D extent) : m_rDevice{ rDevice }, m_windowExtent{ extent }
    {
        ksc_log::debug("Creating swapchain.");

        create_swapchain();
        create_image_views();
        create_render_pass();
        create_depth_resources();
        create_framebuffers();
        create_sync_objects();
    }

    Swapchain::~Swapchain()
    {
        for (auto imageView : m_vSwapchainImageViews)
        {
            vkDestroyImageView(m_rDevice.device(), imageView, nullptr);
        }
        m_vSwapchainImageViews.clear();

        if (m_swapchain != nullptr)
        {
            vkDestroySwapchainKHR(m_rDevice.device(), m_swapchain, nullptr);
            m_swapchain = nullptr;
        }

        for (int i = 0; i < m_vDepthImages.size(); i++)
        {
            vkDestroyImageView(m_rDevice.device(), m_vDepthImageViews[i], nullptr);
            vkDestroyImage(m_rDevice.device(), m_vDepthImages[i], nullptr);
            vkFreeMemory(m_rDevice.device(), m_vDepthImageMemorys[i], nullptr);
        }

        for (auto framebuffer : m_vSwapchainFramebuffers)
        {
            vkDestroyFramebuffer(m_rDevice.device(), framebuffer, nullptr);
        }

        vkDestroyRenderPass(m_rDevice.device(), m_renderPass, nullptr);

        for (size_t i = 0; i < image_count(); i++)
        {
            vkDestroySemaphore(m_rDevice.device(), m_vRenderFinishedSemaphores[i], nullptr);
        }

        // Clean up sync objects
        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            vkDestroySemaphore(m_rDevice.device(), m_vImageAvailableSemaphores[i], nullptr);
            vkDestroyFence(m_rDevice.device(), m_vInFlightFences[i], nullptr);
        }
    }

    VkResult Swapchain::acquire_next_image(uint32_t *pImageIndex)
    {
        vkWaitForFences(m_rDevice.device(), 1, &m_vInFlightFences[m_currentFrame], VK_TRUE, std::numeric_limits<uint64_t>::max());

        VkResult result = vkAcquireNextImageKHR(m_rDevice.device(), m_swapchain, std::numeric_limits<uint64_t>::max(),
            // Can't be a signaled semaphore
            m_vImageAvailableSemaphores[m_currentFrame], VK_NULL_HANDLE, pImageIndex);

        return result;
    }

    VkResult Swapchain::submit_command_buffer(const VkCommandBuffer *pBUFFER, uint32_t *pImageIndex)
    {
        if (m_vImagesInFlight[*pImageIndex] != VK_NULL_HANDLE)
        {
            vkWaitForFences(m_rDevice.device(), 1, &m_vImagesInFlight[*pImageIndex], VK_TRUE, UINT64_MAX);
        }
        m_vImagesInFlight[*pImageIndex] = m_vInFlightFences[m_currentFrame];

        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore pWaitSemaphores[] = { m_vImageAvailableSemaphores[m_currentFrame] };
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = pWaitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;

        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = pBUFFER;

        VkSemaphore pSignalSemaphores[] = { m_vRenderFinishedSemaphores[*pImageIndex] };
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = pSignalSemaphores;

        vkResetFences(m_rDevice.device(), 1, &m_vInFlightFences[m_currentFrame]);
        if (vkQueueSubmit(m_rDevice.graphics_queue(), 1, &submitInfo, m_vInFlightFences[m_currentFrame]) != VK_SUCCESS)
        {
            const char *pErrorMessage = "Failed to submit draw command buffer!";
            ksc_log::error(pErrorMessage);
            throw std::runtime_error(pErrorMessage);
        }

        VkPresentInfoKHR presentInfo = {};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = pSignalSemaphores;

        VkSwapchainKHR pSwapchains[] = { m_swapchain };
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = pSwapchains;

        presentInfo.pImageIndices = pImageIndex;

        auto result = vkQueuePresentKHR(m_rDevice.present_queue(), &presentInfo);

        m_currentFrame = (m_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

        return result;
    }

    void Swapchain::create_swapchain()
    {
        SwapchainSupportDetails swapchainSupport = m_rDevice.get_swapchain_support();

        VkSurfaceFormatKHR surfaceFormat = choose_swapchain_surface_format(swapchainSupport.vFormats);
        VkPresentModeKHR presentMode = choose_swapchain_present_mode(swapchainSupport.vPresentModes);
        VkExtent2D extent = choose_swapchain_extent(swapchainSupport.capabilities);

        uint32_t imageCount = swapchainSupport.capabilities.minImageCount + 1;
        if (swapchainSupport.capabilities.maxImageCount > 0 && imageCount > swapchainSupport.capabilities.maxImageCount)
        {
            imageCount = swapchainSupport.capabilities.maxImageCount;
        }

        VkSwapchainCreateInfoKHR swapchainInfo = {};
        swapchainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        swapchainInfo.surface = m_rDevice.surface();

        swapchainInfo.minImageCount = imageCount;
        swapchainInfo.imageFormat = surfaceFormat.format;
        swapchainInfo.imageColorSpace = surfaceFormat.colorSpace;
        swapchainInfo.imageExtent = extent;
        swapchainInfo.imageArrayLayers = 1;
        swapchainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        QueueFamilyIndices indices = m_rDevice.find_physical_queue_families();
        uint32_t queueFamilyIndices[] = { indices.graphicsFamily, indices.presentFamily };

        if (indices.graphicsFamily != indices.presentFamily)
        {
            swapchainInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            swapchainInfo.queueFamilyIndexCount = 2;
            swapchainInfo.pQueueFamilyIndices = queueFamilyIndices;
        }
        else
        {
            swapchainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            //Optional
            swapchainInfo.queueFamilyIndexCount = 0;
            swapchainInfo.pQueueFamilyIndices = nullptr;
        }

        swapchainInfo.preTransform = swapchainSupport.capabilities.currentTransform;
        swapchainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

        swapchainInfo.presentMode = presentMode;
        swapchainInfo.clipped = VK_TRUE;

        swapchainInfo.oldSwapchain = VK_NULL_HANDLE;

        if (vkCreateSwapchainKHR(m_rDevice.device(), &swapchainInfo, nullptr, &m_swapchain) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create swap chain!");
        }

        // we only specified a minimum number of images in the swap chain, so the implementation is
        // allowed to create a swap chain with more. That's why we'll first query the final number of
        // images with vkGetSwapchainImagesKHR, then resize the container and finally call it again to
        // retrieve the handles.
        vkGetSwapchainImagesKHR(m_rDevice.device(), m_swapchain, &imageCount, nullptr);
        m_vSwapchainImages.resize(imageCount);
        vkGetSwapchainImagesKHR(m_rDevice.device(), m_swapchain, &imageCount, m_vSwapchainImages.data());

        m_swapchainImageFormat = surfaceFormat.format;
        m_swapchainExtent = extent;
    }

    void Swapchain::create_image_views()
    {
        m_vSwapchainImageViews.resize(m_vSwapchainImages.size());
        for (size_t i = 0; i < m_vSwapchainImages.size(); i++)
        {
            VkImageViewCreateInfo viewInfo{};
            viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            viewInfo.image = m_vSwapchainImages[i];
            viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            viewInfo.format = m_swapchainImageFormat;
            viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            viewInfo.subresourceRange.baseMipLevel = 0;
            viewInfo.subresourceRange.levelCount = 1;
            viewInfo.subresourceRange.baseArrayLayer = 0;
            viewInfo.subresourceRange.layerCount = 1;

            if (vkCreateImageView(m_rDevice.device(), &viewInfo, nullptr, &m_vSwapchainImageViews[i]) != VK_SUCCESS)
            {
                const char *pErrorMessage = "Failed to create texture image view!";
                ksc_log::error(pErrorMessage);
                throw std::runtime_error(pErrorMessage);
            }
        }
    }

    void Swapchain::create_render_pass()
    {
        VkAttachmentDescription depthAttachment{};
        depthAttachment.format = find_depth_format();
        depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentReference depthAttachmentRef{};
        depthAttachmentRef.attachment = 1;
        depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentDescription colorAttachment = {};
        colorAttachment.format = get_swapchain_image_format();
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference colorAttachmentRef = {};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass = {};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;
        subpass.pDepthStencilAttachment = &depthAttachmentRef;

        VkSubpassDependency dependency = {};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.srcAccessMask = 0;
        dependency.srcStageMask =
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.dstSubpass = 0;
        dependency.dstStageMask =
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.dstAccessMask =
            VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };
        VkRenderPassCreateInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        renderPassInfo.pAttachments = attachments.data();
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = &dependency;

        if (vkCreateRenderPass(m_rDevice.device(), &renderPassInfo, nullptr, &m_renderPass) != VK_SUCCESS)
        {
            const char *pErrorMessage = "Failed to create render pass!";
            ksc_log::error(pErrorMessage);
            throw std::runtime_error(pErrorMessage);
        }
    }

    void Swapchain::create_framebuffers()
    {
        m_vSwapchainFramebuffers.resize(image_count());
        for (size_t i = 0; i < image_count(); i++)
        {
            std::array<VkImageView, 2> attachments = { m_vSwapchainImageViews[i], m_vDepthImageViews[i] };

            VkExtent2D swapChainExtent = get_swapchain_extent();
            VkFramebufferCreateInfo framebufferInfo = {};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = m_renderPass;
            framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
            framebufferInfo.pAttachments = attachments.data();
            framebufferInfo.width = swapChainExtent.width;
            framebufferInfo.height = swapChainExtent.height;
            framebufferInfo.layers = 1;

            if (vkCreateFramebuffer(m_rDevice.device(), &framebufferInfo, nullptr, &m_vSwapchainFramebuffers[i]) != VK_SUCCESS)
            {
                const char *pErrorMessage = "Failed to create framebuffer!";
                ksc_log::error(pErrorMessage);
                throw std::runtime_error(pErrorMessage);
            }
        }
    }

    void Swapchain::create_depth_resources()
    {
        VkFormat depthFormat = find_depth_format();
        VkExtent2D swapChainExtent = get_swapchain_extent();

        m_vDepthImages.resize(image_count());
        m_vDepthImageMemorys.resize(image_count());
        m_vDepthImageViews.resize(image_count());

        for (int i = 0; i < m_vDepthImages.size(); i++)
        {
            VkImageCreateInfo imageInfo{};
            imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            imageInfo.imageType = VK_IMAGE_TYPE_2D;
            imageInfo.extent.width = swapChainExtent.width;
            imageInfo.extent.height = swapChainExtent.height;
            imageInfo.extent.depth = 1;
            imageInfo.mipLevels = 1;
            imageInfo.arrayLayers = 1;
            imageInfo.format = depthFormat;
            imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
            imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
            imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
            imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            imageInfo.flags = 0;

            m_rDevice.create_image_with_info(imageInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_vDepthImages[i], m_vDepthImageMemorys[i]);

            VkImageViewCreateInfo viewInfo{};
            viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            viewInfo.image = m_vDepthImages[i];
            viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            viewInfo.format = depthFormat;
            viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
            viewInfo.subresourceRange.baseMipLevel = 0;
            viewInfo.subresourceRange.levelCount = 1;
            viewInfo.subresourceRange.baseArrayLayer = 0;
            viewInfo.subresourceRange.layerCount = 1;

            if (vkCreateImageView(m_rDevice.device(), &viewInfo, nullptr, &m_vDepthImageViews[i]) != VK_SUCCESS)
            {
                const char *pErrorMessage = "Failed to create texture image view!";
                ksc_log::error(pErrorMessage);
                throw std::runtime_error(pErrorMessage);
            }
        }
    }

    void Swapchain::create_sync_objects()
    {
        m_vImageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        m_vRenderFinishedSemaphores.resize(image_count());
        m_vInFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
        m_vImagesInFlight.resize(image_count(), VK_NULL_HANDLE);

        VkSemaphoreCreateInfo semaphoreInfo = {};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo = {};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (size_t i = 0; i < image_count(); i++)
        {
            if (vkCreateSemaphore(m_rDevice.device(), &semaphoreInfo, nullptr, &m_vRenderFinishedSemaphores[i]) != VK_SUCCESS)
            {
                std::string errorMessage = std::format("Failed to create sync objects for frame {}!", m_currentFrame);
                ksc_log::error(errorMessage);
                throw std::runtime_error(errorMessage);
            }
        }

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            if (vkCreateSemaphore(m_rDevice.device(), &semaphoreInfo, nullptr, &m_vImageAvailableSemaphores[i]) != VK_SUCCESS ||
                   vkCreateFence(m_rDevice.device(), &fenceInfo, nullptr, &m_vInFlightFences[i]) != VK_SUCCESS)
            {
                std::string errorMessage = std::format("Failed to create sync objects for frame {}!", m_currentFrame);
                ksc_log::error(errorMessage);
                throw std::runtime_error(errorMessage);
            }
        }
    }

    VkSurfaceFormatKHR Swapchain::choose_swapchain_surface_format(const std::vector<VkSurfaceFormatKHR> &rAVAILABLE_FORMATS)
    {
        for (const auto &availableFormat : rAVAILABLE_FORMATS)
        {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            {
                return availableFormat;
            }
        }

        return rAVAILABLE_FORMATS[0];
    }

    VkPresentModeKHR Swapchain::choose_swapchain_present_mode(const std::vector<VkPresentModeKHR> &rAVAILABLE_PRESENT_MODES)
    {
        for (const auto &availablePresentMode : rAVAILABLE_PRESENT_MODES)
        {
            if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
            {
                ksc_log::debug("Present mode: Mailbox");
                return availablePresentMode;
            }
        }

        /*for (const auto &availablePresentMode : rAVAILABLE_PRESENT_MODES)
        {
            if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR)
            {
                ksc_log::debug("Present mode: Immediate");
                return availablePresentMode;
            }
        }*/

        // FIFO = VSYNC
        ksc_log::debug("Present mode: VSYNC");
        return VK_PRESENT_MODE_FIFO_KHR;
    }

    VkExtent2D Swapchain::choose_swapchain_extent(const VkSurfaceCapabilitiesKHR &rCAPABILITIES) const
    {
        if (rCAPABILITIES.currentExtent.width != std::numeric_limits<uint32_t>::max())
        {
            return rCAPABILITIES.currentExtent;
        }
        else
        {
            VkExtent2D actualExtent = m_windowExtent;
            actualExtent.width = std::max(rCAPABILITIES.minImageExtent.width, 
                                 std::min(rCAPABILITIES.maxImageExtent.width, actualExtent.width));

            actualExtent.height = std::max(rCAPABILITIES.minImageExtent.height,
                                  std::min(rCAPABILITIES.maxImageExtent.height, actualExtent.height));

            return actualExtent;
        }
    }

    VkFormat Swapchain::find_depth_format()
    {
        return m_rDevice.find_supported_format(
            // Candidates
            { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
            // Feature flags
            VK_IMAGE_TILING_OPTIMAL,
            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
    }
}