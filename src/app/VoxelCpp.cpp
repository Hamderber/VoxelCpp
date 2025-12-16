#include <ksc_log.hpp>
#include "VoxelCpp/app/App.hpp"

const char *pAPP_NAME = "VoxelCpp";

int main()
{
	App::App VoxelCpp(pAPP_NAME);

	try
	{
		// Actual program
		ksc_log::info("<< Actual program here >>");

		VoxelCpp.loop();
	}
	catch (...)
	{
		// Crash handler for high-level exceptions. The application still crashes, but this
		// allows for graceful crash handling
	}
}