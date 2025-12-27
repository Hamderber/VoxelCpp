#pragma once
#include <vulkan/vulkan_core.h>
#include <unordered_map>
#include <memory>

namespace Rendering { class Device; }

namespace Rendering
{
#pragma region Descriptor Set Layout
    class DescriptorSetLayout
    {
    public:
        class Builder
        {
        public:
            Builder(Device &rDevice) : m_rDevice{ rDevice } { }

            Builder &binding_add(uint32_t binding, VkDescriptorType descriptorType, VkShaderStageFlags stageFlags, uint32_t count = 1);
            std::unique_ptr<DescriptorSetLayout> build() const;

        private:
            Device &m_rDevice;
            std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> m_mBindings{};
        };

        DescriptorSetLayout(Device &rDevice, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> mBindings);
        ~DescriptorSetLayout();

        DescriptorSetLayout(const DescriptorSetLayout &) = delete;
        DescriptorSetLayout &operator=(const DescriptorSetLayout &) = delete;

        VkDescriptorSetLayout descriptor_set_layout_get() const { return m_descriptorSetLayout; }

    private:
        Device &m_rDevice;
        VkDescriptorSetLayout m_descriptorSetLayout;
        std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> m_mBindings;

        friend class DescriptorWriter;
    };
#pragma endregon
#pragma region Descriptor Pool
    class DescriptorPool
    {
    public:
        class Builder
        {
        public:
            Builder(Device &rDevice) : m_rDevice{ rDevice } { }

            Builder &pool_size_add(VkDescriptorType descriptorType, uint32_t count);
            Builder &pool_flags_set(VkDescriptorPoolCreateFlags flags);
            Builder &max_sets_set(uint32_t count);
            std::unique_ptr<DescriptorPool> build() const;

        private:
            Device &m_rDevice;
            std::vector<VkDescriptorPoolSize> m_vPoolSizes{};
            uint32_t m_maxSets = 1000;
            VkDescriptorPoolCreateFlags m_poolFlags = 0;
        };

        DescriptorPool(Device &rDevice, uint32_t maxSets, VkDescriptorPoolCreateFlags poolFlags, 
                          const std::vector<VkDescriptorPoolSize> &rPOOL_SIZES);
        ~DescriptorPool();

        DescriptorPool(const DescriptorPool &) = delete;
        DescriptorPool &operator=(const DescriptorPool &) = delete;

        bool allocate_descriptor_set(const VkDescriptorSetLayout DESC_SET_LAYOUT, VkDescriptorSet &rDescriptors) const;
        void free_descriptor_sets(std::vector<VkDescriptorSet> &rDescriptors) const;
        void pool_reset();

    private:
        Device &m_rDevice;
        VkDescriptorPool m_descriptorPool;

        friend class DescriptorWriter;
    };
#pragma endregion
#pragma region Descriptor Writer
    class DescriptorWriter
    {
    public:
        DescriptorWriter(DescriptorSetLayout &rSetLayout, DescriptorPool &rPool);

        DescriptorWriter &buffer_write(uint32_t binding, VkDescriptorBufferInfo *pBufferInfo);
        DescriptorWriter &image_write(uint32_t binding, VkDescriptorImageInfo *pImageInfo);

        bool build(VkDescriptorSet &rSet);
        void overwrite(VkDescriptorSet &rSet);

    private:
        DescriptorSetLayout &m_rSetLayout;
        DescriptorPool &m_rPool;
        std::vector<VkWriteDescriptorSet> m_vWrites;
    };
#pragma endregion
}