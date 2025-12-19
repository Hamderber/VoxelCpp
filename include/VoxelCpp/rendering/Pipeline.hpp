#pragma once
#include <string>
#include <vector>
#include <vulkan/vulkan_core.h>
#include <cstdint>

namespace Rendering { class Device; }

namespace Rendering
{
	struct PipelineConfigInfo
	{
		VkViewport viewport;
		VkRect2D scissor;
		VkPipelineViewportStateCreateInfo viewportInfo;
		VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
		VkPipelineRasterizationStateCreateInfo rasterizationInfo;
		VkPipelineMultisampleStateCreateInfo multisampleInfo;
		VkPipelineColorBlendAttachmentState colorBlendAttachment;
		VkPipelineColorBlendStateCreateInfo colorBlendInfo;
		VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
		VkPipelineLayout pipelineLayout = nullptr;
		VkRenderPass renderPass = nullptr;
		uint32_t subpass = 0;
	};

	class Pipeline
	{
	public:
		Pipeline(Device &rDevice, const std::string &rVERT_PATH, const std::string &rFRAG_PATH, const PipelineConfigInfo &rCONFIG_INFO);
		~Pipeline();

		Pipeline(const Pipeline &) = delete;
		void operator=(const Pipeline &) = delete;

		static PipelineConfigInfo default_pipeline_config_info(uint32_t width, uint32_t height);

	private:
		static std::vector<char> readFile(const std::string &rPATH);

		void create_graphics_pipeline(const std::string &rVERT_PATH, const std::string &rFRAG_PATH, const PipelineConfigInfo &rCONFIG_INFO);

		void create_shader_module(const std::vector<char> &rCODE, VkShaderModule *pShaderModule);

		Device &m_rDevice;
		VkPipeline m_graphicsPipeline;
		VkShaderModule m_vertShaderModule;
		VkShaderModule m_fragShaderModule;
	};
}