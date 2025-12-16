#pragma once

#include <string>
#include <vector>

namespace Rendering
{
	class Pipeline
	{
	public:
		Pipeline();
		Pipeline(const std::string &rVERT_PATH, const std::string &rFRAG_PATH);
		~Pipeline();

	private:
		static std::vector<char> readFile(const std::string &rPATH);

		void create_graphics_pipeline(const std::string &rVERT_PATH, const std::string &rFRAG_PATH);
	};
}