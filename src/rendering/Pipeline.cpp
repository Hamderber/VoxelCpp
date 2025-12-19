#include <VoxelCpp/rendering/Pipeline.hpp>
#include <ksc_log.hpp>
#include <string>
#include <vector>
#include <fstream>
#include <stdexcept>
#include <format>
#include <VoxelCpp/rendering/Device.hpp>
#include <VoxelCpp/rendering/Model.hpp>
#include <vulkan/vulkan_core.h>
#include <cstdint>
#include <cassert>

namespace Rendering
{
	Pipeline::Pipeline(Device &rDevice, const std::string &rVERT_PATH, const std::string &rFRAG_PATH,
					   const PipelineConfigInfo &rCONFIG_INFO) : m_rDevice{ rDevice }
	{
		ksc_log::debug("Creating pipeline.");
		create_graphics_pipeline(rVERT_PATH, rFRAG_PATH, rCONFIG_INFO);
	}

	Pipeline::~Pipeline()
	{
		ksc_log::debug("Destroying pipeline.");
		
		vkDestroyShaderModule(m_rDevice.device(), m_vertShaderModule, nullptr);
		vkDestroyShaderModule(m_rDevice.device(), m_fragShaderModule, nullptr);
		vkDestroyPipeline(m_rDevice.device(), m_graphicsPipeline, nullptr);
	}

	void Pipeline::default_pipeline_config_info(PipelineConfigInfo &rConfigInfo, uint32_t width, uint32_t height)
	{
		rConfigInfo.inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		rConfigInfo.inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		rConfigInfo.inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

		rConfigInfo.viewport.x = 0.0f;
		rConfigInfo.viewport.y = 0.0f;
		rConfigInfo.viewport.width = static_cast<float>(width);
		rConfigInfo.viewport.height = static_cast<float>(height);
		rConfigInfo.viewport.minDepth = 0.0f;
		rConfigInfo.viewport.maxDepth = 1.0f;

		rConfigInfo.scissor.offset = { 0, 0 };
		rConfigInfo.scissor.extent = { width, height };

		rConfigInfo.viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		rConfigInfo.viewportInfo.viewportCount = 1;
		rConfigInfo.viewportInfo.pViewports = &rConfigInfo.viewport;
		rConfigInfo.viewportInfo.scissorCount = 1;
		rConfigInfo.viewportInfo.pScissors = &rConfigInfo.scissor;

		rConfigInfo.rasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rConfigInfo.rasterizationInfo.depthClampEnable = VK_FALSE;
		rConfigInfo.rasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
		rConfigInfo.rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
		rConfigInfo.rasterizationInfo.lineWidth = 1.0f;
		rConfigInfo.rasterizationInfo.cullMode = VK_CULL_MODE_NONE;
		rConfigInfo.rasterizationInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
		rConfigInfo.rasterizationInfo.depthBiasEnable = VK_FALSE;

		// Optional
		rConfigInfo.rasterizationInfo.depthBiasConstantFactor = 0.0f;
		rConfigInfo.rasterizationInfo.depthBiasClamp = 0.0f;
		rConfigInfo.rasterizationInfo.depthBiasSlopeFactor = 0.0f;

		rConfigInfo.multisampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		rConfigInfo.multisampleInfo.sampleShadingEnable = VK_FALSE;
		rConfigInfo.multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

		// Optional
		rConfigInfo.multisampleInfo.minSampleShading = 1.0f;
		rConfigInfo.multisampleInfo.pSampleMask = nullptr;
		rConfigInfo.multisampleInfo.alphaToCoverageEnable = VK_FALSE;
		rConfigInfo.multisampleInfo.alphaToOneEnable = VK_FALSE;

		rConfigInfo.colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
			VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		rConfigInfo.colorBlendAttachment.blendEnable = VK_FALSE;

		// Optional
		rConfigInfo.colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
		rConfigInfo.colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
		rConfigInfo.colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
		rConfigInfo.colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		rConfigInfo.colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		rConfigInfo.colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

		rConfigInfo.colorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		rConfigInfo.colorBlendInfo.logicOpEnable = VK_FALSE;
		rConfigInfo.colorBlendInfo.attachmentCount = 1;
		rConfigInfo.colorBlendInfo.pAttachments = &rConfigInfo.colorBlendAttachment;

		// Optional
		rConfigInfo.colorBlendInfo.logicOp = VK_LOGIC_OP_COPY;
		rConfigInfo.colorBlendInfo.blendConstants[0] = 0.0f;
		rConfigInfo.colorBlendInfo.blendConstants[1] = 0.0f;
		rConfigInfo.colorBlendInfo.blendConstants[2] = 0.0f;
		rConfigInfo.colorBlendInfo.blendConstants[3] = 0.0f;

		rConfigInfo.depthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		rConfigInfo.depthStencilInfo.depthTestEnable = VK_TRUE;
		rConfigInfo.depthStencilInfo.depthWriteEnable = VK_TRUE;
		rConfigInfo.depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS;
		rConfigInfo.depthStencilInfo.stencilTestEnable = VK_FALSE;
		rConfigInfo.depthStencilInfo.depthBoundsTestEnable = VK_FALSE;

		// Optional
		rConfigInfo.depthStencilInfo.minDepthBounds = 0.0f;
		rConfigInfo.depthStencilInfo.maxDepthBounds = 1.0f;
		rConfigInfo.depthStencilInfo.front = {};
		rConfigInfo.depthStencilInfo.back = {};
	}

	void Pipeline::bind(VkCommandBuffer commandBuffer) const
	{
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphicsPipeline);
	}

	std::vector<char> Pipeline::read_file(const std::string &rPATH)
	{
		std::ifstream file{ rPATH, std::ios::ate | std::ios::binary };
		
		if (!file.is_open())
		{
			std::string errorMessage = "Failed to open file: " + rPATH;
			ksc_log::error(errorMessage);
			throw std::runtime_error(errorMessage);
		}

		size_t fileSize = static_cast<size_t>(file.tellg());
		std::vector<char> vBuffer(fileSize);
		
		file.seekg(0);
		file.read(vBuffer.data(), fileSize);
		file.close();

		return vBuffer;
	}

	void Pipeline::create_graphics_pipeline(const std::string &rVERT_PATH, const std::string &rFRAG_PATH,
											const PipelineConfigInfo &rCONFIG_INFO)
	{
		assert(rCONFIG_INFO.pipelineLayout != VK_NULL_HANDLE &&
			   "Cannot create graphics pipeline: no pipelineLayout provided in the config info.");
		assert(rCONFIG_INFO.pipelineLayout != VK_NULL_HANDLE &&
			   "Cannot create graphics pipeline: no renderPass provided in the config info.");

		auto vertCode = read_file(rVERT_PATH);
		auto fragCode = read_file(rFRAG_PATH);

		create_shader_module(vertCode, &m_vertShaderModule);
		ksc_log::debug(std::format("Created vertex shader with a code size of {} bytes.", vertCode.size()));

		create_shader_module(fragCode, &m_fragShaderModule);
		ksc_log::debug(std::format("Created fragment shader with a code size of {} byes.", fragCode.size()));

		const char *pSHADER_ENTRY_FUNCTION_NAME = "main";
		constexpr uint16_t STAGE_COUNT = 2;

		VkPipelineShaderStageCreateInfo pShaderStages[STAGE_COUNT]{};

		// Vertex
		pShaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		pShaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
		pShaderStages[0].module = m_vertShaderModule;
		pShaderStages[0].pName = pSHADER_ENTRY_FUNCTION_NAME;
		pShaderStages[0].flags = 0;
		pShaderStages[0].pNext = nullptr;
		pShaderStages[0].pSpecializationInfo = nullptr;

		// Fragment
		pShaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		pShaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		pShaderStages[1].module = m_fragShaderModule;
		pShaderStages[1].pName = pSHADER_ENTRY_FUNCTION_NAME;
		pShaderStages[1].flags = 0;
		pShaderStages[1].pNext = nullptr;
		pShaderStages[1].pSpecializationInfo = nullptr;
	

		auto attributeDescriptions = Model::Vertex::get_attribute_descriptions();
		auto bindingDescriptions = Model::Vertex::get_binding_descriptions();

		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		// Hardcoded points for hello world
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
		vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
		vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescriptions.size());
		vertexInputInfo.pVertexBindingDescriptions = bindingDescriptions.data();

		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = STAGE_COUNT;
		pipelineInfo.pStages = pShaderStages;
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &rCONFIG_INFO.inputAssemblyInfo;
		pipelineInfo.pViewportState = &rCONFIG_INFO.viewportInfo;
		pipelineInfo.pRasterizationState = &rCONFIG_INFO.rasterizationInfo;
		pipelineInfo.pMultisampleState = &rCONFIG_INFO.multisampleInfo;
		pipelineInfo.pColorBlendState = &rCONFIG_INFO.colorBlendInfo;
		pipelineInfo.pDepthStencilState = &rCONFIG_INFO.depthStencilInfo;
		// Optional
		pipelineInfo.pDynamicState = nullptr;

		pipelineInfo.layout = rCONFIG_INFO.pipelineLayout;
		pipelineInfo.renderPass = rCONFIG_INFO.renderPass;
		pipelineInfo.subpass = rCONFIG_INFO.subpass;

		// Consider using for performance (future) because it is cheaper to create a new pipeline by derriving from
		// an existing one
		pipelineInfo.basePipelineIndex = -1;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
	
		constexpr uint32_t PIPELINE_COUNT = 1;
		if (vkCreateGraphicsPipelines(m_rDevice.device(), VK_NULL_HANDLE, PIPELINE_COUNT, &pipelineInfo, nullptr, &m_graphicsPipeline)
			!= VK_SUCCESS)
		{
			const char *pErrorMessage = "Failed to create graphics pipeline!";
			ksc_log::error(pErrorMessage);
			throw std::runtime_error(pErrorMessage);
		}
	}

	void Pipeline::create_shader_module(const std::vector<char> &rCODE, VkShaderModule *pShaderModule)
	{
		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = rCODE.size();
		createInfo.pCode = reinterpret_cast<const uint32_t *>(rCODE.data());

		if (vkCreateShaderModule(m_rDevice.device(), &createInfo, nullptr, pShaderModule) != VK_SUCCESS)
		{
			const char *pErrorMessage = "Failed to create shader module!";
			ksc_log::error(pErrorMessage);
			throw std::runtime_error(pErrorMessage);
		}
	}
}