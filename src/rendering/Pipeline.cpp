#include <VoxelCpp/rendering/Pipeline.hpp>
#include <ksc_log.hpp>
#include <string>
#include <vector>
#include <fstream>
#include <stdexcept>
#include <format>
#include <VoxelCpp/rendering/Device.hpp>
#include <vulkan/vulkan_core.h>
#include <cstdint>

namespace Rendering
{
	Pipeline::Pipeline(Device &rDevice, const std::string &rVERT_PATH, const std::string &rFRAG_PATH,
					   const PipelineConfigInfo &rCONFIG_INFO) : m_rDevice{rDevice}
	{
		ksc_log::debug("Creating pipeline.");
		create_graphics_pipeline(rVERT_PATH, rFRAG_PATH, rCONFIG_INFO);
	}

	Pipeline::~Pipeline()
	{
		ksc_log::debug("Destroying pipeline.");
	}

	PipelineConfigInfo Pipeline::default_pipeline_config_info(uint32_t width, uint32_t height)
	{
		PipelineConfigInfo configInfo{};
		return configInfo;
	}

	std::vector<char> Pipeline::readFile(const std::string &rPATH)
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
		auto vertCode = readFile(rVERT_PATH);
		auto fragCode = readFile(rFRAG_PATH);

		ksc_log::debug(std::format("Vertex shader code size: {}", vertCode.size()));
		ksc_log::debug(std::format("Fragment shader code size: {}", fragCode.size()));
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