#include <ksc_log.hpp>
#include "VoxelCpp/app/App.hpp"

int main()
{
	App::start();

	// Actual program
	ksc_log::info("<< Actual program here >>");

	App::end();
}