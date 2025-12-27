#include <ksc_log.hpp>
#include <VoxelCpp/app/ProgramConstants.hpp>
#include <VoxelCpp/rendering/Renderer.hpp>
#include <VoxelCpp/rendering/Device.hpp>
#include <VoxelCpp/rendering/RenderSystem.hpp>
#include <VoxelCpp/rendering/Pipeline.hpp>
#include <VoxelCpp/game/GameObject.hpp>
#include <VoxelCpp/rendering/Camera.hpp>
#include <stdexcept>
#include <vector>
#include <vulkan/vulkan_core.h>
#include <memory>
#include <glm/gtc/constants.hpp>
#include <cstdint>

namespace Rendering
{
	RenderSystem::RenderSystem(Device &rDevice, VkRenderPass renderPass, VkDescriptorSetLayout globalDescSetLayout) : m_rDevice{ rDevice }
	{
		ksc_log::debug("Creating RenderSystem...");
		pipeline_layout_create(globalDescSetLayout);
		pipeline_create(renderPass);
	}

	RenderSystem::~RenderSystem()
	{
		ksc_log::debug("Destroying RenderSystem...");
		pipeline_destroy();
	}

	void RenderSystem::game_objects_render(FrameInfo &rFrameInfo, std::vector<Game::GameObject> &vGameObjects)
	{
		m_pPipeline->bind(rFrameInfo.commandBuffer);

		vkCmdBindDescriptorSets(rFrameInfo.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, 0, 1,
								&rFrameInfo.globalDescSet, 0, nullptr);

		for (auto &go : vGameObjects)
		{
			Rendering::SimplePushConstantData push{};
			auto modelMatrix = go.transform.matrix();
			push.modelMatrix = modelMatrix;

			vkCmdPushConstants((rFrameInfo.commandBuffer), m_pipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT,
							   0, static_cast<uint32_t>(sizeof(Rendering::SimplePushConstantData)), &push);

			go.pModel->bind(rFrameInfo.commandBuffer);
			go.pModel->draw(rFrameInfo.commandBuffer);
		}
	}

	void RenderSystem::pipeline_layout_create(VkDescriptorSetLayout globalDescSetLayout)
	{
		VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(SimplePushConstantData);

		std::vector<VkDescriptorSetLayout> vDescSetLayouts{globalDescSetLayout};


		VkPipelineLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		layoutInfo.setLayoutCount = static_cast<uint32_t>(vDescSetLayouts.size());
		layoutInfo.pSetLayouts = vDescSetLayouts.data();
		layoutInfo.pushConstantRangeCount = 1;
		layoutInfo.pPushConstantRanges = &pushConstantRange;

		if (vkCreatePipelineLayout(m_rDevice.device(), &layoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS)
		{
			const char *pErrorMessage = "Failed to create pipeline layout!";
			ksc_log::error(pErrorMessage);
			throw std::runtime_error(pErrorMessage);
		}
	}

	void RenderSystem::pipeline_create(VkRenderPass renderPass)
	{
		assert(m_pipelineLayout && "Cannot create the pipeline before the pipeline layout!");

		auto root = ProgramConstants::root_filepath();

		PipelineConfigInfo pipelineConfig{};
		Pipeline::default_pipeline_config_info(pipelineConfig);
		pipelineConfig.renderPass = renderPass;
		pipelineConfig.pipelineLayout = m_pipelineLayout;
		m_pPipeline = std::make_unique<Rendering::Pipeline>(m_rDevice,
															(root / "res/shaders/simple_shader.vert.spv").generic_string(),
															(root / "res/shaders/simple_shader.frag.spv").generic_string(),
															pipelineConfig);
	}

	void RenderSystem::pipeline_destroy()
	{
		vkDestroyPipelineLayout(m_rDevice.device(), m_pipelineLayout, nullptr);
	}
}