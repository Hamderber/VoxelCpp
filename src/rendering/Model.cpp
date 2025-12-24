#include <VoxelCpp/rendering/Model.hpp>
#include <VoxelCpp/rendering/Device.hpp>
#include <vector>
#include <vulkan/vulkan_core.h>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <cstddef>

namespace Rendering
{
	Model::Model(Device &rDevice, const std::vector<Vertex> &rVERTICIES) : m_rDevice { rDevice }
	{
		create_vertex_buffers(rVERTICIES);
	}

	Model::~Model()
	{
		vkDestroyBuffer(m_rDevice.device(), m_vertexBuffer, nullptr);
		vkFreeMemory(m_rDevice.device(), m_vertexBufferMemory, nullptr);
	}

	void Model::bind(VkCommandBuffer commandBuffer)
	{
		constexpr uint32_t BUFFER_COUNT = 1;
		VkBuffer pBuffers[BUFFER_COUNT] = { m_vertexBuffer };
		VkDeviceSize pOffsets[BUFFER_COUNT] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, BUFFER_COUNT, pBuffers, pOffsets);
	}

	void Model::draw(VkCommandBuffer commandBuffer) const
	{
		vkCmdDraw(commandBuffer, m_vertexCount, 1, 0, 0);
	}

	void Model::create_vertex_buffers(const std::vector<Vertex> &rVERTICIES)
	{
		m_vertexCount = static_cast<uint32_t>(rVERTICIES.size());
		assert(m_vertexCount >= 3 && "Vertex count must be at least 3 (to make a triangle).");
		
		size_t bufferSize = sizeof(rVERTICIES[0]) * m_vertexCount;
		m_rDevice.create_buffer(bufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
								VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
								m_vertexBuffer, m_vertexBufferMemory);

		void *pData;
		vkMapMemory(m_rDevice.device(), m_vertexBufferMemory, 0, bufferSize, 0, &pData);
		memcpy(pData, rVERTICIES.data(), bufferSize);
		vkUnmapMemory(m_rDevice.device(), m_vertexBufferMemory);
	}

	std::vector<VkVertexInputBindingDescription> Model::Vertex::get_binding_descriptions()
	{
		std::vector<VkVertexInputBindingDescription> vBindingDescriptions(1);

		vBindingDescriptions[0].binding = 0;
		vBindingDescriptions[0].stride = sizeof(Vertex);
		vBindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return vBindingDescriptions;
	}

	std::vector<VkVertexInputAttributeDescription> Model::Vertex::get_attribute_descriptions()
	{
		std::vector<VkVertexInputAttributeDescription> vAttributeDescriptions(2);
		vAttributeDescriptions[0].binding = 0;
		vAttributeDescriptions[0].location = 0;
		vAttributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		vAttributeDescriptions[0].offset = offsetof(Vertex, position);

		vAttributeDescriptions[1].binding = 0;
		vAttributeDescriptions[1].location = 1;
		vAttributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		vAttributeDescriptions[1].offset = offsetof(Vertex, color);

		return vAttributeDescriptions;
	}
}