#include <ksc_log.hpp>

namespace App
{
	static void start_logging(void)
	{
		std::filesystem::path root = std::filesystem::current_path() / ".." / ".." / "..";

		const bool USE_TIMESTAMP = true;
		ksc_log::begin("VoxelCpp", root, USE_TIMESTAMP, ksc_log::Level::Debug);
	}

	static void end_logging(void) { ksc_log::end(); }

	void start(void)
	{
		start_logging();
	}

	void end(void)
	{
		end_logging();
	}
}