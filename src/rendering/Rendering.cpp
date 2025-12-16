#include <VoxelCpp/app/App.hpp>
#include <ksc_log.hpp>
#include <VoxelCpp/rendering/Rendering.hpp>
#include <memory>
#include <VoxelCpp/rendering/Window.hpp>
#include <VoxelCpp/rendering/Pipeline.hpp>

namespace Rendering
{
	Rendering::Rendering(App::App &rApp) : m_rApp{rApp}
	{
		ksc_log::debug("Rendering created.");

		m_Window = std::make_unique<Window>(800, 600, m_rApp.get_name());

		m_Pipeline = Pipeline(
			(m_rApp.get_root() / "res/shaders/simple_shader.vert.spv").generic_string(),
			(m_rApp.get_root() / "res/shaders/simple_shader.frag.spv").generic_string());
	}

	Rendering::~Rendering()
	{
		ksc_log::debug("Destroying rendering...");
	}

	void Rendering::loop()
	{
		//ksc_log::debug("Render loop");
	}
}