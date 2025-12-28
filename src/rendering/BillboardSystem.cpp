#include <ksc_log.hpp>
#include <VoxelCpp/app/ProgramConstants.hpp>
#include <VoxelCpp/rendering/BillboardSystem.hpp>
#include <VoxelCpp/rendering/FrameInfo.hpp>
#include <VoxelCpp/rendering/RenderSystem.hpp>
#include <VoxelCpp/rendering/Device.hpp>

namespace Rendering
{
	BillboardSystem::BillboardSystem(Device &rDevice, VkRenderPass renderPass, VkDescriptorSetLayout globalDescSetLayout) : m_rDevice{ rDevice }
	{
		ksc_log::debug("Creating BillboardSystem...");
		pipeline_layout_create(globalDescSetLayout);
		pipeline_create(renderPass);
	}

	BillboardSystem::~BillboardSystem()
	{
		ksc_log::debug("Destroying BillboardSystem...");
		pipeline_destroy();
	}

	void BillboardSystem::render(FrameInfo &rFrameInfo)
	{
		m_pPipeline->bind(rFrameInfo.commandBuffer);

		vkCmdBindDescriptorSets(rFrameInfo.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, 0, 1,
								&rFrameInfo.globalDescSet, 0, nullptr);

		// Square billboard test
		vkCmdDraw(rFrameInfo.commandBuffer, 6, 1, 0, 0);
	}

	void BillboardSystem::pipeline_layout_create(VkDescriptorSetLayout globalDescSetLayout)
	{
		//VkPushConstantRange pushConstantRange{};
		//pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		//pushConstantRange.offset = 0;
		//pushConstantRange.size = sizeof(SimplePushConstantData);

		std::vector<VkDescriptorSetLayout> vDescSetLayouts{ globalDescSetLayout };


		VkPipelineLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		layoutInfo.setLayoutCount = static_cast<uint32_t>(vDescSetLayouts.size());
		layoutInfo.pSetLayouts = vDescSetLayouts.data();
		layoutInfo.pushConstantRangeCount = 0;
		layoutInfo.pPushConstantRanges = nullptr;

		if (vkCreatePipelineLayout(m_rDevice.device(), &layoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS)
		{
			const char *pErrorMessage = "Failed to create pipeline layout!";
			ksc_log::error(pErrorMessage);
			throw std::runtime_error(pErrorMessage);
		}
	}

	void BillboardSystem::pipeline_create(VkRenderPass renderPass)
	{
		assert(m_pipelineLayout && "Cannot create the pipeline before the pipeline layout!");

		auto root = ProgramConstants::root_filepath();

		PipelineConfigInfo pipelineConfig{};
		Pipeline::default_pipeline_config_info(pipelineConfig);
		
		// Defaults are for models so clear them (unused here)
		pipelineConfig.vBindingDescs.clear();
		pipelineConfig.vAttributeDescs.clear();

		pipelineConfig.renderPass = renderPass;
		pipelineConfig.pipelineLayout = m_pipelineLayout;
		m_pPipeline = std::make_unique<Rendering::Pipeline>(m_rDevice,
															(root / "res/shaders/billboard_shader.vert.spv").generic_string(),
															(root / "res/shaders/billboard_shader.frag.spv").generic_string(),
															pipelineConfig);
	}

	void BillboardSystem::pipeline_destroy()
	{
		vkDestroyPipelineLayout(m_rDevice.device(), m_pipelineLayout, nullptr);
	}
}