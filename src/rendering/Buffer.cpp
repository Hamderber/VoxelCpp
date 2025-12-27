#include <VoxelCpp/rendering/Buffer.hpp>
#include <VoxelCpp/rendering/Device.hpp>
#include <cassert>

namespace Rendering
{
    VkDeviceSize Buffer::alignment_get(VkDeviceSize instanceSize, VkDeviceSize minOffsetAlignment)
    {
        return minOffsetAlignment > 0 ? (instanceSize + minOffsetAlignment - 1) & ~(minOffsetAlignment - 1) : instanceSize;
    }

    Buffer::Buffer(Device &device, VkDeviceSize instanceSize, uint32_t instanceCount, VkBufferUsageFlags usageFlags,
                   VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize minOffsetAlignment) : 
        m_rDevice{ device }, m_instanceSize{ instanceSize }, m_instanceCount{ instanceCount }, m_usageFlags{ usageFlags }, 
        m_memoryPropertyFlags{ memoryPropertyFlags }
    {
        m_alignmentSize = alignment_get(instanceSize, minOffsetAlignment);
        m_bufferSize = m_alignmentSize * instanceCount;
        device.create_buffer(m_bufferSize, usageFlags, memoryPropertyFlags, m_buffer, m_memory);
    }

    Buffer::~Buffer()
    {
        unmap();
        vkDestroyBuffer(m_rDevice.device(), m_buffer, nullptr);
        vkFreeMemory(m_rDevice.device(), m_memory, nullptr);
    }

    VkResult Buffer::map(VkDeviceSize size, VkDeviceSize offset)
    {
        assert(m_buffer && m_memory && "Called map on buffer before it was created!");
        return vkMapMemory(m_rDevice.device(), m_memory, offset, size, 0, &m_pMapped);
    }

    void Buffer::unmap()
    {
        if (m_pMapped)
        {
            vkUnmapMemory(m_rDevice.device(), m_memory);
            m_pMapped = nullptr;
        }
    }

    void Buffer::write_to_buffer(void *pData, VkDeviceSize size, VkDeviceSize offset)
    {
        assert(m_pMapped && "Cannot copy data to an unmapped buffer!");

        if (size == VK_WHOLE_SIZE)
        {
            memcpy(m_pMapped, pData, m_bufferSize);
        }
        else
        {
            char *pMemOffset = (char *)m_pMapped;
            pMemOffset += offset;
            memcpy(pMemOffset, pData, size);
        }
    }

    VkResult Buffer::flush(VkDeviceSize size, VkDeviceSize offset)
    {
        VkMappedMemoryRange mappedRange{};
        mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
        mappedRange.memory = m_memory;
        mappedRange.offset = offset;
        mappedRange.size = size;
        return vkFlushMappedMemoryRanges(m_rDevice.device(), 1, &mappedRange);
    }

    VkResult Buffer::invalidate(VkDeviceSize size, VkDeviceSize offset)
    {
        VkMappedMemoryRange mappedRange{};
        mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
        mappedRange.memory = m_memory;
        mappedRange.offset = offset;
        mappedRange.size = size;
        return vkInvalidateMappedMemoryRanges(m_rDevice.device(), 1, &mappedRange);
    }

    VkDescriptorBufferInfo Buffer::descriptor_info(VkDeviceSize size, VkDeviceSize offset)
    {
        return VkDescriptorBufferInfo{
            m_buffer,
            offset,
            size,
        };
    }

    void Buffer::write_to_index(void *pData, int index)
    {
        write_to_buffer(pData, m_instanceSize, index * m_alignmentSize);
    }

    VkResult Buffer::flush_index(int index) { return flush(m_alignmentSize, index * m_alignmentSize); }

    VkDescriptorBufferInfo Buffer::index_descriptor_info(int index)
    {
        return descriptor_info(m_alignmentSize, index * m_alignmentSize);
    }

    VkResult Buffer::invalidate_index(int index)
    {
        return invalidate(m_alignmentSize, index * m_alignmentSize);
    }
}