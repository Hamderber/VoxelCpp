#include <VoxelCpp/rendering/Pipeline.hpp>
#include <ksc_log.hpp>
#include <string>
#include <vector>
#include <fstream>
#include <stdexcept>
#include <format>

namespace Rendering
{
	Pipeline::Pipeline()
	{

	}

	Pipeline::Pipeline(const std::string &rVERT_PATH, const std::string &rFRAG_PATH)
	{
		ksc_log::debug("Creating pipeline.");
		create_graphics_pipeline(rVERT_PATH, rFRAG_PATH);
	}

	Pipeline::~Pipeline()
	{
		ksc_log::debug("Destroying pipeline.");
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
		std::vector<char> buffer(fileSize);
		
		file.seekg(0);
		file.read(buffer.data(), fileSize);
		file.close();

		return buffer;
	}

	void Pipeline::create_graphics_pipeline(const std::string &rVERT_PATH, const std::string &rFRAG_PATH)
	{
		auto vertCode = readFile(rVERT_PATH);
		auto fragCode = readFile(rFRAG_PATH);

		ksc_log::debug(std::format("Vertex shader code size: {}", vertCode.size()));
		ksc_log::debug(std::format("Fragment shader code size: {}", fragCode.size()));
	}
}