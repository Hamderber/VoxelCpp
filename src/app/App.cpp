#include <VoxelCpp/app/ProgramConstants.hpp>
#include <filesystem>
#include <VoxelCpp/app/App.hpp>
#include <ksc_log.hpp>
#include <VoxelCpp/rendering/Rendering.hpp>
#include <VoxelCpp/rendering/Window.hpp>
#include <memory>

namespace App
{
	App::App()
	{
		m_root = std::filesystem::current_path() / ".." / ".." / "..";

		start_logging();
		ksc_log::debug("App created. Root: " + m_root.generic_string());

		rendering = std::make_unique<Rendering::Rendering>(*this);
	}

	App::~App()
	{
		ksc_log::debug("Destroying app...");
		ksc_log::end();
	}

	void App::start_logging()
	{
		const bool USE_TIMESTAMP = true;
		ksc_log::begin(APPLICATION_NAME, m_root, USE_TIMESTAMP, ksc_log::Level::Debug);
	}

	void App::loop(void) const
	{
		// Breaks when the glfw window should close
		do
		{
			// App happens here.
			//ksc_log::debug("App loop");

			rendering->loop();
		} while (!rendering->window.poll_or_exit());
		
		// Put this somewhere else maybe?
		// device wait idle on exit. like a rendering cleanup func?
	}

	std::filesystem::path App::get_root()
	{
		return m_root;
	}
}