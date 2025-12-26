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
	RenderSystem::RenderSystem(Device &rDevice, VkRenderPass renderPass) : m_rDevice{ rDevice }
	{
		ksc_log::debug("Creating RenderSystem...");
		pipeline_layout_create();
		pipeline_create(renderPass);
	}

	RenderSystem::~RenderSystem()
	{
		ksc_log::debug("Destroying RenderSystem...");
		pipeline_destroy();
	}

	void RenderSystem::game_objects_render(VkCommandBuffer commandBuffer, std::vector<Game::GameObject> &vGameObjects, const Camera &rCAMERA)
	{
		m_pPipeline->bind(commandBuffer);

		auto projectionView = rCAMERA.projection_get() * rCAMERA.view_get();

		for (auto &go : vGameObjects)
		{
			Rendering::SimplePushConstantData push{};
			push.color = go.color;
			push.transform = projectionView * go.transform.matrix();

			vkCmdPushConstants((commandBuffer), m_pipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT,
							   0, static_cast<uint32_t>(sizeof(Rendering::SimplePushConstantData)), &push);

			go.pModel->bind(commandBuffer);
			go.pModel->draw(commandBuffer);
		}
	}

	void RenderSystem::pipeline_layout_create()
	{
		VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(SimplePushConstantData);

		VkPipelineLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		layoutInfo.setLayoutCount = 0;
		layoutInfo.pSetLayouts = nullptr;
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