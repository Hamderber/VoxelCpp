#include <VoxelCpp/rendering/Device.hpp>
#include <ksc_log.hpp>
#include <format>
#include <vulkan/vk_platform.h>
#include <vulkan/vulkan_core.h>
#include <VoxelCpp/rendering/Window.hpp>
#include <string.h>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>
#include <GLFW/glfw3.h>
#include <set>
#include <unordered_set>

namespace Rendering
{
    static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT *pCALLBACK_DATA,
        void *pUserData)
    {
        ksc_log::error(std::format("Validation layer: {}", pCALLBACK_DATA->pMessage));

        return VK_FALSE;
    }

    static VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCREATE_INFO,
        const VkAllocationCallbacks *pALLOCATOR, VkDebugUtilsMessengerEXT *pDebugMessenger)
    {
        auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");

        if (func != nullptr)
        {
            return func(instance, pCREATE_INFO, pALLOCATOR, pDebugMessenger);
        }
        else
        {
            return VK_ERROR_EXTENSION_NOT_PRESENT;
        }
    }

    static void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger,
                                              const VkAllocationCallbacks *pALLOCATOR)
    {
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");

        if (func != nullptr)
        {
            func(instance, debugMessenger, pALLOCATOR);
        }
    }

    Device::Device(Window &rWindow) : m_rWindow { rWindow }
    {
        create_instance();
        setup_debug_messenger();
        create_surface();
        pick_physical_device();
        create_logical_device();
        create_command_pool();
    }

    Device::~Device()
    {
        vkDestroyCommandPool(m_device, m_commandPool, nullptr);
        vkDestroyDevice(m_device, nullptr);

        if (ENABLE_VALIDATION_LAYERS)
        {
            DestroyDebugUtilsMessengerEXT(m_instance, m_debugMessenger, nullptr);
        }

        vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
        vkDestroyInstance(m_instance, nullptr);
    }

    void Device::create_instance()
    {
        if (ENABLE_VALIDATION_LAYERS && !check_validation_layer_support())
        {
            const char *pErrorMessage = "Validation layers requested but not available!";
            ksc_log::error(pErrorMessage);
            throw std::runtime_error(pErrorMessage);
        }

        VkApplicationInfo appInfo = {};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Application";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_0;

        VkInstanceCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;

        auto extensions = get_required_extensions();
        createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        createInfo.ppEnabledExtensionNames = extensions.data();

        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
        if (ENABLE_VALIDATION_LAYERS)
        {
            createInfo.enabledLayerCount = static_cast<uint32_t>(m_vValidationLayers.size());
            createInfo.ppEnabledLayerNames = m_vValidationLayers.data();

            populate_debug_messenger_create_info(debugCreateInfo);
            createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT *)&debugCreateInfo;
        }
        else
        {
            createInfo.enabledLayerCount = 0;
            createInfo.pNext = nullptr;
        }

        if (vkCreateInstance(&createInfo, nullptr, &m_instance) != VK_SUCCESS)
        {
            const char *pErrorMessage = "Failed to create Vulkan instance!";
            ksc_log::error(pErrorMessage);
            throw std::runtime_error(pErrorMessage);
        }

        has_gflw_required_instance_extensions();
    }

    void Device::pick_physical_device()
    {
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(m_instance, &deviceCount, nullptr);
        if (deviceCount == 0)
        {
            const char *pErrorMessage = "Failed to find GPUs with Vulkan support!";
            ksc_log::error(pErrorMessage);
            throw std::runtime_error(pErrorMessage);
        }
        ksc_log::debug(std::format("Device count: {}", deviceCount));
        std::vector<VkPhysicalDevice> vDevices(deviceCount);
        vkEnumeratePhysicalDevices(m_instance, &deviceCount, vDevices.data());

        for (const auto &device : vDevices)
        {
            if (is_device_suitable(device))
            {
                m_physicalDevice = device;
                break;
            }
        }

        if (m_physicalDevice == VK_NULL_HANDLE)
        {
            const char *pErrorMessage = "Failed to find a suitable GPU!";
            ksc_log::error(pErrorMessage);
            throw std::runtime_error(pErrorMessage);
        }

        vkGetPhysicalDeviceProperties(m_physicalDevice, &properties);
        ksc_log::debug(std::format("Physical device name: {}", properties.deviceName));
    }

    void Device::create_logical_device()
    {
        QueueFamilyIndices indices = find_queue_families(m_physicalDevice);

        std::vector<VkDeviceQueueCreateInfo> vQueueCreateInfos;
        std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily, indices.presentFamily };

        float queuePriority = 1.0f;
        for (uint32_t queueFamily : uniqueQueueFamilies)
        {
            VkDeviceQueueCreateInfo queueCreateInfo = {};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;
            vQueueCreateInfos.push_back(queueCreateInfo);
        }

        VkPhysicalDeviceFeatures deviceFeatures = {};
        deviceFeatures.samplerAnisotropy = VK_TRUE;

        VkDeviceCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

        createInfo.queueCreateInfoCount = static_cast<uint32_t>(vQueueCreateInfos.size());
        createInfo.pQueueCreateInfos = vQueueCreateInfos.data();

        createInfo.pEnabledFeatures = &deviceFeatures;
        createInfo.enabledExtensionCount = static_cast<uint32_t>(m_vDeviceExtensions.size());
        createInfo.ppEnabledExtensionNames = m_vDeviceExtensions.data();

        if (vkCreateDevice(m_physicalDevice, &createInfo, nullptr, &m_device) != VK_SUCCESS)
        {
            const char *pErrorMessage = "Failed to create logical Vulkan device!";
            ksc_log::error(pErrorMessage);
            throw std::runtime_error(pErrorMessage);
        }

        vkGetDeviceQueue(m_device, indices.graphicsFamily, 0, &m_graphicsQueue);
        vkGetDeviceQueue(m_device, indices.presentFamily, 0, &m_presentQueue);
    }

    void Device::create_command_pool()
    {
        QueueFamilyIndices queueFamilyIndices = find_physical_queue_families();

        VkCommandPoolCreateInfo poolInfo = {};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

        if (vkCreateCommandPool(m_device, &poolInfo, nullptr, &m_commandPool) != VK_SUCCESS)
        {
            const char *pErrorMessage = "Failed to create command pool!";
            ksc_log::error(pErrorMessage);
            throw std::runtime_error(pErrorMessage);
        }
    }

    void Device::create_surface() { m_rWindow.create_surface(m_instance, &m_surface); }

    bool Device::is_device_suitable(VkPhysicalDevice device)
    {
        QueueFamilyIndices indices = find_queue_families(device);

        bool extensionsSupported = check_device_extension_support(device);

        bool swapChainAdequate = false;
        if (extensionsSupported)
        {
            SwapchainSupportDetails swapchainSupport = query_swapchain_support(device);
            swapChainAdequate = !swapchainSupport.vFormats.empty() && !swapchainSupport.vPresentModes.empty();
        }

        VkPhysicalDeviceFeatures supportedFeatures;
        vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

        return indices.is_complete() && extensionsSupported && swapChainAdequate &&
            supportedFeatures.samplerAnisotropy;
    }

    void Device::populate_debug_messenger_create_info(VkDebugUtilsMessengerCreateInfoEXT &createInfo)
    {
        createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = DebugCallback;
    }

    void Device::setup_debug_messenger()
    {
        if (!ENABLE_VALIDATION_LAYERS) return;

        VkDebugUtilsMessengerCreateInfoEXT createInfo;
        populate_debug_messenger_create_info(createInfo);

        if (CreateDebugUtilsMessengerEXT(m_instance, &createInfo, nullptr, &m_debugMessenger) != VK_SUCCESS)
        {
            const char *pErrorMessage = "Failed to setup Vulkan debug messenger!";
            ksc_log::error(pErrorMessage);
            throw std::runtime_error(pErrorMessage);
        }
    }

    bool Device::check_validation_layer_support()
    {
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

        std::vector<VkLayerProperties> vAvailableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, vAvailableLayers.data());

        for (const char *layerName : m_vValidationLayers)
        {
            bool layerFound = false;

            for (const auto &layerProperties : vAvailableLayers)
            {
                if (strcmp(layerName, layerProperties.layerName) == 0)
                {
                    layerFound = true;
                    break;
                }
            }

            if (!layerFound)
            {
                return false;
            }
        }

        return true;
    }

    std::vector<const char *> Device::get_required_extensions() const
    {
        uint32_t glfwExtensionCount = 0;
        const char **ppGLFWExtensions;
        ppGLFWExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        std::vector<const char *> vExtensions(ppGLFWExtensions, ppGLFWExtensions + glfwExtensionCount);

        if (ENABLE_VALIDATION_LAYERS)
        {
            vExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        return vExtensions;
    }

    void Device::has_gflw_required_instance_extensions()
    {
        uint32_t extensionCount = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
        std::vector<VkExtensionProperties> vExtensions(extensionCount);
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, vExtensions.data());

        ksc_log::debug("Available extensions:");
        std::unordered_set<std::string> available;
        for (const auto &extension : vExtensions)
        {
            ksc_log::debug(std::format("\t{}", extension.extensionName));
            available.insert(extension.extensionName);
        }

        ksc_log::debug("Required extensions:");
        auto requiredExtensions = get_required_extensions();
        for (const auto &required : requiredExtensions)
        {
            ksc_log::debug(std::format("\t{}", required));
            if (available.find(required) == available.end())
            {
                const char *pErrorMessage = "Missing required GLFW extension!";
                ksc_log::error(pErrorMessage);
                throw std::runtime_error(pErrorMessage);
            }
        }
    }

    bool Device::check_device_extension_support(VkPhysicalDevice device)
    {
        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> vAvailableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(
            device,
            nullptr,
            &extensionCount,
            vAvailableExtensions.data());

        std::set<std::string> requiredExtensions(m_vDeviceExtensions.begin(), m_vDeviceExtensions.end());

        for (const auto &extension : vAvailableExtensions)
        {
            requiredExtensions.erase(extension.extensionName);
        }

        return requiredExtensions.empty();
    }

    QueueFamilyIndices Device::find_queue_families(VkPhysicalDevice device) const
    {
        QueueFamilyIndices indices;

        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> vQueueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, vQueueFamilies.data());

        int i = 0;
        for (const auto &queueFamily : vQueueFamilies)
        {
            if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
                indices.graphicsFamily = i;
                indices.graphicsFamilyHasValue = true;
            }
            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_surface, &presentSupport);
            if (queueFamily.queueCount > 0 && presentSupport)
            {
                indices.presentFamily = i;
                indices.presentFamilyHasValue = true;
            }
            if (indices.is_complete())
            {
                break;
            }

            i++;
        }

        return indices;
    }

    SwapchainSupportDetails Device::query_swapchain_support(VkPhysicalDevice device) const
    {
        SwapchainSupportDetails details;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_surface, &details.capabilities);

        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &formatCount, nullptr);

        if (formatCount != 0)
        {
            details.vFormats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &formatCount, details.vFormats.data());
        }

        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &presentModeCount, nullptr);

        if (presentModeCount != 0)
        {
            details.vPresentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(
                device,
                m_surface,
                &presentModeCount,
                details.vPresentModes.data());
        }

        return details;
    }

    VkFormat Device::find_supported_format(const std::vector<VkFormat> &rCANDIDATES, VkImageTiling tiling, VkFormatFeatureFlags features) const
    {
        for (VkFormat format : rCANDIDATES)
        {
            VkFormatProperties props;
            vkGetPhysicalDeviceFormatProperties(m_physicalDevice, format, &props);

            if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
            {
                return format;
            }
            else if (
                tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
            {
                return format;
            }
        }

        const char *pErrorMessage = "Failed to find supported format!";
        ksc_log::error(pErrorMessage);
        throw std::runtime_error(pErrorMessage);
    }

    uint32_t Device::find_memory_type(uint32_t typeFilter, VkMemoryPropertyFlags properties) const
    {
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &memProperties);
        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
        {
            if ((typeFilter & (1 << i)) &&
                (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
            {
                return i;
            }
        }

        const char *pErrorMessage = "Failed to find suitable memory type!";
        ksc_log::error(pErrorMessage);
        throw std::runtime_error(pErrorMessage);
    }

    void Device::create_buffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer &rBuffer, 
                              VkDeviceMemory &rBufferMemory) const
    {
        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateBuffer(m_device, &bufferInfo, nullptr, &rBuffer) != VK_SUCCESS)
        {
            const char *pErrorMessage = "Failed to create vertex buffer!";
            ksc_log::error(pErrorMessage);
            throw std::runtime_error(pErrorMessage);
        }

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(m_device, rBuffer, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = find_memory_type(memRequirements.memoryTypeBits, properties);

        if (vkAllocateMemory(m_device, &allocInfo, nullptr, &rBufferMemory) != VK_SUCCESS)
        {
            const char *pErrorMessage = "Failed to allocate vertex buffer memory!";
            ksc_log::error(pErrorMessage);
            throw std::runtime_error(pErrorMessage);
        }

        vkBindBufferMemory(m_device, rBuffer, rBufferMemory, 0);
    }

    VkCommandBuffer Device::begin_single_time_commands() const
    {
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = m_commandPool;
        allocInfo.commandBufferCount = 1;

        VkCommandBuffer commandBuffer;
        vkAllocateCommandBuffers(m_device, &allocInfo, &commandBuffer);

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(commandBuffer, &beginInfo);

        return commandBuffer;
    }

    void Device::end_single_time_commands(VkCommandBuffer commandBuffer) const
    {
        vkEndCommandBuffer(commandBuffer);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);

        vkQueueWaitIdle(m_graphicsQueue);

        vkFreeCommandBuffers(m_device, m_commandPool, 1, &commandBuffer);
    }

    void Device::copy_buffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) const
    {
        VkCommandBuffer commandBuffer = begin_single_time_commands();

        VkBufferCopy copyRegion{};
        copyRegion.size = size;
        vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

        end_single_time_commands(commandBuffer);
    }

    void Device::copy_buffer_to_image(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t layerCount) const
    {
        VkCommandBuffer commandBuffer = begin_single_time_commands();

        VkBufferImageCopy region{};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;

        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = layerCount;

        region.imageOffset = { 0, 0, 0 };
        region.imageExtent = { width, height, 1 };

        vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
        end_single_time_commands(commandBuffer);
    }

    void Device::create_image_with_info(const VkImageCreateInfo &rIMAGE_INFO, VkMemoryPropertyFlags properties, VkImage &rImage,
                                        VkDeviceMemory &rImageMemory) const
    {
        if (vkCreateImage(m_device, &rIMAGE_INFO, nullptr, &rImage) != VK_SUCCESS)
        {
            const char *pErrorMessage = "Failed to create image!";
            ksc_log::error(pErrorMessage);
            throw std::runtime_error(pErrorMessage);
        }

        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(m_device, rImage, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = find_memory_type(memRequirements.memoryTypeBits, properties);

        if (vkAllocateMemory(m_device, &allocInfo, nullptr, &rImageMemory) != VK_SUCCESS)
        {
            const char *pErrorMessage = "Failed to allocate image memory!";
            ksc_log::error(pErrorMessage);
            throw std::runtime_error(pErrorMessage);
        }

        if (vkBindImageMemory(m_device, rImage, rImageMemory, 0) != VK_SUCCESS)
        {
            const char *pErrorMessage = "Failed to bind image memory!";
            ksc_log::error(pErrorMessage);
            throw std::runtime_error(pErrorMessage);
        }
    }
}