#pragma once
#include <vulkan/vulkan_core.h>

namespace Rendering { class Device; }

namespace Rendering
{
	class Buffer
	{
    public:
        Buffer(Device &rDevice, VkDeviceSize instanceSize, uint32_t instanceCount, VkBufferUsageFlags usageFlags, 
               VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize minOffsetAlignment = 1);
        ~Buffer();

        Buffer(const Buffer &) = delete;
        Buffer &operator=(const Buffer &) = delete;

        VkResult map(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
        void unmap();

        void write_to_buffer(void *pData, VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
        VkResult flush(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
        VkDescriptorBufferInfo descriptor_info(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
        VkResult invalidate(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);

        void write_to_index(void *pData, int index);
        VkResult flush_index(int index);
        VkDescriptorBufferInfo index_descriptor_info(int index);
        VkResult invalidate_index(int index);

        VkBuffer buffer_get() const { return m_buffer; }
        void *mapped_memory_get() const { return m_pMapped; }
        uint32_t instance_count_get() const { return m_instanceCount; }
        VkDeviceSize instance_size_get() const { return m_instanceSize; }
        VkDeviceSize alignment_size_get() const { return m_instanceSize; }
        VkBufferUsageFlags usage_flags_get() const { return m_usageFlags; }
        VkMemoryPropertyFlags memory_property_flags_get() const { return m_memoryPropertyFlags; }
        VkDeviceSize buffer_size_get() const { return m_bufferSize; }

    private:
        static VkDeviceSize alignment_get(VkDeviceSize instanceSize, VkDeviceSize minOffsetAlignment);

        Device &m_rDevice;
        void *m_pMapped = nullptr;
        VkBuffer m_buffer = VK_NULL_HANDLE;
        VkDeviceMemory m_memory = VK_NULL_HANDLE;

        VkDeviceSize m_bufferSize;
        uint32_t m_instanceCount;
        VkDeviceSize m_instanceSize;
        VkDeviceSize m_alignmentSize;
        VkBufferUsageFlags m_usageFlags;
        VkMemoryPropertyFlags m_memoryPropertyFlags;
    };
}