#include <ksc_log.hpp>
#include "VoxelCpp/app/App.hpp"
#include <VoxelCpp/app/ProgramConstants.hpp>
#include <filesystem>

static void start_logging(std::filesystem::path root)
{
	const bool USE_TIMESTAMP = true;
	ksc_log::begin(ProgramConstants::APP_NAME, root, USE_TIMESTAMP, ksc_log::Level::Debug);
	ksc_log::debug("Application start (main() START).");
}

static void end_logging()
{
	ksc_log::debug("Application end (main() END).");
	ksc_log::end();
}

int main()
{
	auto root = ProgramConstants::root_filepath();
	start_logging(root);

	App::App VoxelCpp(root);

	ksc_log::debug("<< Actual program here start >>");

	try
	{
		VoxelCpp.loop();
	}
	catch (...)
	{
		// Crash handler for high-level exceptions. The application still crashes, but this
		// allows for graceful crash handling
	}

	ksc_log::debug("<< Actual program here end >>");

	end_logging();
}