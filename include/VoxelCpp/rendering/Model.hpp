#pragma once
#include <vulkan/vulkan_core.h>
#include <cstdint>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/fwd.hpp>
#include <vector>
#include <memory>
#include <string>

namespace Rendering { class Device; };

namespace Rendering
{
	class Model
	{
	public:

		struct Vertex
		{
			glm::vec3 position{};
			glm::vec3 color{};
			glm::vec3 normal{};
			glm::vec2 uv{};

			static std::vector<VkVertexInputBindingDescription> get_binding_descriptions();
			static std::vector<VkVertexInputAttributeDescription> get_attribute_descriptions();

			bool operator==(const Vertex &rOTHER) const
			{
				return position == rOTHER.position && color == rOTHER.color && normal == rOTHER.normal && uv == rOTHER.uv;
			}
		};

		struct Builder
		{
			std::vector<Vertex> vVerticies{};
			std::vector<uint32_t> vIndices{};

			void load_model(const std::string &rFILE_PATH);
		};

		Model(Device &rDevice, const Builder &rBUILDER);
		~Model();

		Model(const Model &) = delete;
		Model &operator=(const Model &) = delete;

		static std::unique_ptr<Model> create_model_from_file(Device &rDevice, const std::string &rFILE_PATH);

		void bind(VkCommandBuffer commandBuffer);
		void draw(VkCommandBuffer commandBuffer) const;

	private:
		void create_vertex_buffer(const std::vector<Vertex> &rVERTICIES);
		void create_index_buffer(const std::vector<uint32_t> &rINDICIES);

		Device &m_rDevice;

		VkBuffer m_vertexBuffer;
		VkDeviceMemory m_vertexBufferMemory;
		uint32_t m_vertexCount;
	
		bool m_hasIndexBuffer = false;
		VkBuffer m_indexBuffer;
		VkDeviceMemory m_indexBufferMemory;
		uint32_t m_indexCount;
	};
}