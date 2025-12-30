#pragma once
#include <vulkan/vulkan_core.h>
#include <memory>
#include <VoxelCpp/rendering/Pipeline.hpp>
#include <VoxelCpp/input/KeyboardMovementController.hpp>
#include <glm/glm.hpp>
#include <glm/fwd.hpp>
#include "FrameInfo.hpp"

namespace Rendering { class Device; class FrameInfo; }

namespace Rendering
{
	struct PointLightPushConstants
	{
		glm::vec4 position{};
		glm::vec4 color{};
		float radius;
	};

	class BillboardSystem
	{
	public:
		BillboardSystem(Device &rDevice, VkRenderPass renderPass, VkDescriptorSetLayout globalDescSetLayout);
		~BillboardSystem();

		BillboardSystem(const BillboardSystem &) = delete;
		BillboardSystem &operator=(const BillboardSystem &) = delete;

		void update(FrameInfo &rFrameInfo, GlobalUBO &rGlobalUBO);
		void render(FrameInfo &rFrameInfo);

	private:
		void pipeline_layout_create(VkDescriptorSetLayout globalDescSetLayout);
		void pipeline_create(VkRenderPass renderPass);
		void pipeline_destroy();

		Device &m_rDevice;
		std::unique_ptr<Pipeline> m_pPipeline;
		VkPipelineLayout m_pipelineLayout;
	};
}