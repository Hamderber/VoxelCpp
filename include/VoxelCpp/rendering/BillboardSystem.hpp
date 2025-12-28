#pragma once
#include <vulkan/vulkan_core.h>
#include <memory>
#include <VoxelCpp/rendering/Pipeline.hpp>

namespace Rendering { class Device; class FrameInfo; }

namespace Rendering
{
	class BillboardSystem
	{
	public:
		BillboardSystem(Device &rDevice, VkRenderPass renderPass, VkDescriptorSetLayout globalDescSetLayout);
		~BillboardSystem();

		BillboardSystem(const BillboardSystem &) = delete;
		BillboardSystem &operator=(const BillboardSystem &) = delete;

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