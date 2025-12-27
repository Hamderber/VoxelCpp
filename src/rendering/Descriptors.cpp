#include <ksc_log.hpp>
#include <VoxelCpp/rendering/Descriptors.hpp>
#include <cassert>
#include <memory>
#include <VoxelCpp/rendering/Device.hpp>
#include <stdexcept>

namespace Rendering
{
#pragma region Descriptor Set Layout
    DescriptorSetLayout::Builder &DescriptorSetLayout::Builder::binding_add(uint32_t binding, VkDescriptorType descriptorType,
                                                                            VkShaderStageFlags stageFlags, uint32_t count)
    {
        assert(m_mBindings.count(binding) == 0 && "Binding already in use");

        VkDescriptorSetLayoutBinding layoutBinding{};
        layoutBinding.binding = binding;
        layoutBinding.descriptorType = descriptorType;
        layoutBinding.descriptorCount = count;
        layoutBinding.stageFlags = stageFlags;

        m_mBindings[binding] = layoutBinding;

        return *this;
    }

    std::unique_ptr<DescriptorSetLayout> DescriptorSetLayout::Builder::build() const
    {
        return std::make_unique<DescriptorSetLayout>(m_rDevice, m_mBindings);
    }

    DescriptorSetLayout::DescriptorSetLayout(Device &rDevice, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> mBindings) :
        m_rDevice{ rDevice }, m_mBindings{ mBindings }
    {
        std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings{};
        for (auto &kvp : mBindings)
        {
            setLayoutBindings.emplace_back(kvp.second);
        }

        VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo{};
        descriptorSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        descriptorSetLayoutInfo.bindingCount = static_cast<uint32_t>(setLayoutBindings.size());
        descriptorSetLayoutInfo.pBindings = setLayoutBindings.data();

        if (vkCreateDescriptorSetLayout(rDevice.device(), &descriptorSetLayoutInfo, nullptr, &m_descriptorSetLayout) != VK_SUCCESS)
        {
            const char *pErrorMessage = "Failed to create descriptor set layout!";
            ksc_log::error(pErrorMessage);
            throw std::runtime_error(pErrorMessage);
        }
    }

    DescriptorSetLayout::~DescriptorSetLayout()
    {
        vkDestroyDescriptorSetLayout(m_rDevice.device(), m_descriptorSetLayout, nullptr);
    }
#pragma endregion
#pragma region Descriptor Pool
    DescriptorPool::Builder &DescriptorPool::Builder::pool_size_add(VkDescriptorType descriptorType, uint32_t count)
    {
        m_vPoolSizes.emplace_back(descriptorType, count);

        return *this;
    }

    DescriptorPool::Builder &DescriptorPool::Builder::pool_flags_set(VkDescriptorPoolCreateFlags flags)
    {
        m_poolFlags = flags;

        return *this;
    }

    DescriptorPool::Builder &DescriptorPool::Builder::max_sets_set(uint32_t count)
    {
        m_maxSets = count;

        return *this;
    }

    std::unique_ptr<DescriptorPool> DescriptorPool::Builder::build() const
    {
        return std::make_unique<DescriptorPool>(m_rDevice, m_maxSets, m_poolFlags, m_vPoolSizes);
    }

    DescriptorPool::DescriptorPool(Device &rDevice, uint32_t maxSets, VkDescriptorPoolCreateFlags poolFlags, 
                                   const std::vector<VkDescriptorPoolSize> &rPoolSizes) : m_rDevice{ rDevice }
    {
        VkDescriptorPoolCreateInfo descriptorPoolInfo{};
        descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        descriptorPoolInfo.poolSizeCount = static_cast<uint32_t>(rPoolSizes.size());
        descriptorPoolInfo.pPoolSizes = rPoolSizes.data();
        descriptorPoolInfo.maxSets = maxSets;
        descriptorPoolInfo.flags = poolFlags;

        if (vkCreateDescriptorPool(rDevice.device(), &descriptorPoolInfo, nullptr, &m_descriptorPool) != VK_SUCCESS)
        {
            const char *pErrorMessage = "Failed to create descriptor pool!";
            ksc_log::error(pErrorMessage);
            throw std::runtime_error(pErrorMessage);
        }
    }

    DescriptorPool::~DescriptorPool()
    {
        vkDestroyDescriptorPool(m_rDevice.device(), m_descriptorPool, nullptr);
    }

    bool DescriptorPool::allocate_descriptor_set(const VkDescriptorSetLayout DESC_SET_LAYOUT, VkDescriptorSet &rDescriptor) const
    {
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = m_descriptorPool;
        allocInfo.pSetLayouts = &DESC_SET_LAYOUT;
        allocInfo.descriptorSetCount = 1;

        // Might want to create a "DescriptorPoolManager" class that handles this case and builds
        // a new pool whenever an old pool fills up in the future
        if (vkAllocateDescriptorSets(m_rDevice.device(), &allocInfo, &rDescriptor) != VK_SUCCESS)
        {
            return false;
        }

        return true;
    }

    void DescriptorPool::free_descriptor_sets(std::vector<VkDescriptorSet> &rDescriptors) const
    {
        vkFreeDescriptorSets(m_rDevice.device(), m_descriptorPool, static_cast<uint32_t>(rDescriptors.size()), rDescriptors.data());
    }

    void DescriptorPool::pool_reset()
    {
        vkResetDescriptorPool(m_rDevice.device(), m_descriptorPool, 0);
    }
#pragma endregion
#pragma region Descriptor Writer
    DescriptorWriter::DescriptorWriter(DescriptorSetLayout &rSetLayout, DescriptorPool &rPool) : m_rSetLayout{ rSetLayout }, m_rPool{ rPool }
    {

    }

    DescriptorWriter &DescriptorWriter::buffer_write(uint32_t binding, VkDescriptorBufferInfo *pBufferInfo)
    {
        assert(m_rSetLayout.m_mBindings.count(binding) == 1 && "Layout does not contain specified binding");

        auto &bindingDescription = m_rSetLayout.m_mBindings[binding];

        assert(bindingDescription.descriptorCount == 1 && "Binding single descriptor info, but binding expects multiple");

        VkWriteDescriptorSet write{};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.descriptorType = bindingDescription.descriptorType;
        write.dstBinding = binding;
        write.pBufferInfo = pBufferInfo;
        write.descriptorCount = 1;

        m_vWrites.push_back(write);

        return *this;
    }

    DescriptorWriter &DescriptorWriter::image_write(uint32_t binding, VkDescriptorImageInfo *pImageInfo)
    {
        assert(m_rSetLayout.m_mBindings.count(binding) == 1 && "Layout does not contain specified binding");

        auto &bindingDescription = m_rSetLayout.m_mBindings[binding];

        assert(bindingDescription.descriptorCount == 1 && "Binding single descriptor info, but binding expects multiple");

        VkWriteDescriptorSet write{};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.descriptorType = bindingDescription.descriptorType;
        write.dstBinding = binding;
        write.pImageInfo = pImageInfo;
        write.descriptorCount = 1;

        m_vWrites.push_back(write);

        return *this;
    }

    bool DescriptorWriter::build(VkDescriptorSet &rSet)
    {
        bool success = m_rPool.allocate_descriptor_set(m_rSetLayout.descriptor_set_layout_get(), rSet);

        if (!success) return false;

        overwrite(rSet);

        return true;
    }

    void DescriptorWriter::overwrite(VkDescriptorSet &rSet)
    {
        for (auto &write : m_vWrites)
        {
            write.dstSet = rSet;
        }

        vkUpdateDescriptorSets(m_rPool.m_rDevice.device(), m_vWrites.size(), m_vWrites.data(), 0, nullptr);
    }
#pragma endregion
}