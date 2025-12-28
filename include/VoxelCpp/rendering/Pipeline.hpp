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
		PipelineConfigInfo() = default;

		PipelineConfigInfo(const PipelineConfigInfo &) = delete;
		PipelineConfigInfo &operator=(const PipelineConfigInfo &) = delete;

		std::vector<VkVertexInputBindingDescription> vBindingDescs{};
		std::vector<VkVertexInputAttributeDescription> vAttributeDescs{};
		VkPipelineViewportStateCreateInfo viewportInfo{};
		VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo{};
		VkPipelineRasterizationStateCreateInfo rasterizationInfo{};
		VkPipelineMultisampleStateCreateInfo multisampleInfo{};
		VkPipelineColorBlendAttachmentState colorBlendAttachment{};
		VkPipelineColorBlendStateCreateInfo colorBlendInfo{};
		VkPipelineDepthStencilStateCreateInfo depthStencilInfo{};
		std::vector<VkDynamicState> vDynamicStateEnables;
		VkPipelineDynamicStateCreateInfo dynamicStateInfo{};
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
		Pipeline operator=(const Pipeline &) = delete;

		static void default_pipeline_config_info(PipelineConfigInfo &rConfigInfo);
		void bind(VkCommandBuffer commandBuffer) const;

	private:
		static std::vector<char> read_file(const std::string &rPATH);
		void create_graphics_pipeline(const std::string &rVERT_PATH, const std::string &rFRAG_PATH, const PipelineConfigInfo &rCONFIG_INFO);
		void create_shader_module(const std::vector<char> &rCODE, VkShaderModule *pShaderModule);

		Device &m_rDevice;
		VkPipeline m_graphicsPipeline;
		VkShaderModule m_vertShaderModule;
		VkShaderModule m_fragShaderModule;
	};
}