#include <ksc_log.hpp>
#include <VoxelCpp/rendering/Model.hpp>
#include <VoxelCpp/rendering/Device.hpp>
#include <vector>
#include <vulkan/vulkan_core.h>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <cstddef>
#define TINYOBJLOADER_IMPLEMENTATION
#include <tinyobjectloader/tiny_obj_loader.h>
#include <memory>
#include <stdexcept>
#include <VoxelCpp/util/ObjectHashing.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>
#include <unordered_map>

namespace std
{
	template<>
	struct hash<Rendering::Model::Vertex>
	{
		size_t operator()(Rendering::Model::Vertex const &rVERTEX) const
		{
			size_t seed{};
			Util::hash_combine(seed, rVERTEX.position, rVERTEX.color, rVERTEX.normal, rVERTEX.uv);
			return seed;
		}
	};
}

namespace Rendering
{
	Model::Model(Device &rDevice, const Builder &rBUILDER) : m_rDevice { rDevice }
	{
		create_vertex_buffer(rBUILDER.vVerticies);
		create_index_buffer(rBUILDER.vIndices);
	}

	Model::~Model()
	{

	}

	std::unique_ptr<Model> Model::create_model_from_file(Device &rDevice, const std::string &rFILE_PATH)
	{
		Builder builder{};
		builder.load_model(rFILE_PATH);

		ksc_log::debug(std::format("Model loaded. Vertex count: {}", builder.vVerticies.size()));

		return std::make_unique<Model>(rDevice, builder);
	}

	void Model::bind(VkCommandBuffer commandBuffer)
	{
		constexpr uint32_t BUFFER_COUNT = 1;
		VkBuffer pBuffers[BUFFER_COUNT] = { m_pVertexBuffer->buffer_get() };
		VkDeviceSize pOffsets[BUFFER_COUNT] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, BUFFER_COUNT, pBuffers, pOffsets);

		if (m_hasIndexBuffer)
		{
			// Consider adding a model validation path that changes the index buffer type and member variable to be uint_16t
			// if there aren't any models loaded that exceed the 65k threshold.
			vkCmdBindIndexBuffer(commandBuffer, m_pIndexBuffer->buffer_get(), 0, VK_INDEX_TYPE_UINT32);
		}
	}

	void Model::draw(VkCommandBuffer commandBuffer) const
	{
		if (m_hasIndexBuffer)
		{
			vkCmdDrawIndexed(commandBuffer, m_indexCount, 1, 0, 0, 0);
		}
		else
		{
			vkCmdDraw(commandBuffer, m_vertexCount, 1, 0, 0);
		}
	}

	void Model::create_vertex_buffer(const std::vector<Vertex> &rVERTICIES)
	{
		m_vertexCount = static_cast<uint32_t>(rVERTICIES.size());
		assert(m_vertexCount >= 3 && "Vertex count must be at least 3 (to make a triangle).");
		
		VkDeviceSize bufferSize = sizeof(rVERTICIES[0]) * m_vertexCount;

		uint32_t vertexSize = sizeof(rVERTICIES[0]);
		Buffer stagingBuffer{
			m_rDevice,
			vertexSize,
			m_vertexCount,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT};

		stagingBuffer.map();
		stagingBuffer.write_to_buffer((void *)rVERTICIES.data());

		m_pVertexBuffer = std::make_unique<Buffer>(
			m_rDevice,
			vertexSize,
			m_vertexCount,
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		m_rDevice.copy_buffer(stagingBuffer.buffer_get(), m_pVertexBuffer->buffer_get(), bufferSize);
	}

	void Model::create_index_buffer(const std::vector<uint32_t> &rINDICIES)
	{
		m_indexCount = static_cast<uint32_t>(rINDICIES.size());
		m_hasIndexBuffer = m_indexCount > 0;

		if (!m_hasIndexBuffer) return;

		VkDeviceSize bufferSize = sizeof(rINDICIES[0]) * m_indexCount;
		uint32_t indexSize = sizeof(rINDICIES[0]);

		Buffer stagingBuffer{
			m_rDevice,
			indexSize,
			m_indexCount,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT};

		stagingBuffer.map();
		stagingBuffer.write_to_buffer((void *)rINDICIES.data());

		m_pIndexBuffer = std::make_unique<Buffer>(
			m_rDevice,
			indexSize,
			m_indexCount,
			VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		m_rDevice.copy_buffer(stagingBuffer.buffer_get(), m_pIndexBuffer->buffer_get(), bufferSize);
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
		std::vector<VkVertexInputAttributeDescription> vAttributeDescriptions{};

		// Location | Binding | Format | Offset				    (What this description is for) vvv
		vAttributeDescriptions.emplace_back(0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position));
		vAttributeDescriptions.emplace_back(1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color));
		vAttributeDescriptions.emplace_back(2, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, normal));
		vAttributeDescriptions.emplace_back(3, 0, VK_FORMAT_R32G32_SFLOAT,    offsetof(Vertex, uv));

		return vAttributeDescriptions;
	}

	void Model::Builder::load_model(const std::string &rFILE_PATH)
	{
		ksc_log::debug("Loading model from path " + rFILE_PATH);

		// TODO: Consider baseline reserve size?

		// Position, color, normal, texture coordinate data
		tinyobj::attrib_t attrib;
		// Index values for each face element
		std::vector<tinyobj::shape_t> shapes;
		// Object surface materials
		std::vector<tinyobj::material_t> materials;
		// Populated by tinyObjLoader (C str)
		std::string warn, error;

		if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &error, rFILE_PATH.c_str()))
		{
			std::string errorMessage = warn + error;
			ksc_log::error(errorMessage);
			throw std::runtime_error(errorMessage);
		}

		vVerticies.clear();
		vIndices.clear();

		std::unordered_map<Vertex, uint32_t> mUniqueVerticies{};

		for (const auto &shape : shapes)
		{
			for (const auto &index : shape.mesh.indices)
			{
				Vertex vertex{};

				// Negative means no index was provided (it's optional)
				if (index.vertex_index >= 0)
				{
					vertex.position = {
						attrib.vertices[3 * index.vertex_index + 0],
						attrib.vertices[3 * index.vertex_index + 1],
						attrib.vertices[3 * index.vertex_index + 2],
					};

					vertex.color = {
						attrib.colors[3 * index.vertex_index + 0],
						attrib.colors[3 * index.vertex_index + 1],
						attrib.colors[3 * index.vertex_index + 2],
					};
				}

				if (index.normal_index >= 0)
				{
					vertex.normal = {
						attrib.normals[3 * index.normal_index + 0],
						attrib.normals[3 * index.normal_index + 1],
						attrib.normals[3 * index.normal_index + 2],
					};
				}

				if (index.texcoord_index >= 0)
				{
					vertex.uv = {
						attrib.texcoords[2 * index.texcoord_index + 0],
						attrib.texcoords[2 * index.texcoord_index + 1],
					};
				}

				if (mUniqueVerticies.count(vertex) == 0)
				{
					mUniqueVerticies[vertex] = static_cast<uint32_t>(vVerticies.size());
					vVerticies.push_back(vertex);
				}
				vIndices.push_back(mUniqueVerticies[vertex]);
			}
		}
	}
}