#include <filesystem>
#include <VoxelCpp/app/App.hpp>
#include <ksc_log.hpp>
#include <VoxelCpp/rendering/Rendering.hpp>
#include <memory>

namespace App
{
	App::App(const char *pNAME) : m_pAPP_NAME{pNAME}
	{
		m_root = std::filesystem::current_path() / ".." / ".." / "..";

		start_logging(m_pAPP_NAME);
		ksc_log::debug("App created. Root: " + m_root.generic_string());

		m_Rendering = std::make_unique<Rendering::Rendering>(*this);
	}

	App::~App()
	{
		ksc_log::debug("Destroying app...");
		ksc_log::end();
	}

	void App::start_logging(const char *pAPP_NAME)
	{
		const bool USE_TIMESTAMP = true;
		ksc_log::begin(pAPP_NAME, m_root, USE_TIMESTAMP, ksc_log::Level::Debug);
	}

	void App::loop(void) const
	{
		// Breaks when the glfw window should close
		do
		{
			// App happens here.
			//ksc_log::debug("App loop");

			m_Rendering->loop();
		} while (!m_Rendering->m_Window->poll_or_exit());
	}

	std::filesystem::path App::get_root()
	{
		return m_root;
	}

	const char *App::get_name() const
	{
		return m_pAPP_NAME;
	}
}