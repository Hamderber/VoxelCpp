#pragma once
#include <vulkan/vulkan_core.h>
#include <cstdint>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/fwd.hpp>
#include <vector>

namespace Rendering { class Device; };

namespace Rendering
{
	class Model
	{
	public:

		struct Vertex
		{
			glm::vec2 position;
			glm::vec3 color;

			static std::vector<VkVertexInputBindingDescription> get_binding_descriptions();
			static std::vector<VkVertexInputAttributeDescription> get_attribute_descriptions();
		};

		Model(Device &rDevice, const std::vector<Vertex> &rVERTICIES);
		~Model();

		Model(const Model &) = delete;
		Model &operator=(const Model &) = delete;

		void bind(VkCommandBuffer commandBuffer);
		void draw(VkCommandBuffer commandBuffer) const;

	private:
		void create_vertex_buffers(const std::vector<Vertex> &rVERTICIES);

		Device &m_rDevice;
		VkBuffer m_vertexBuffer;
		VkDeviceMemory m_vertexBufferMemory;
		uint32_t m_vertexCount;
	};
}