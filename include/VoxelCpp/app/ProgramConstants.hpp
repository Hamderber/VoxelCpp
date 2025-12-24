#pragma once
#include <filesystem>

namespace ProgramConstants
{
	auto constexpr APP_NAME = "VoxelCpp";
	auto constexpr DEFAULT_WINDOW_WIDTH = 800;
	auto constexpr DEFAULT_WINDOW_HEIGHT = 600;
	
	static const std::filesystem::path root_filepath()
	{
		return std::filesystem::current_path() / ".." / ".." / "..";
	}
}