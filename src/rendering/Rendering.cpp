#include <VoxelCpp/app/ProgramConstants.hpp>
#include <VoxelCpp/app/App.hpp>
#include <ksc_log.hpp>
#include <VoxelCpp/rendering/Rendering.hpp>
#include <VoxelCpp/rendering/Window.hpp>
#include <VoxelCpp/rendering/Pipeline.hpp>

namespace Rendering
{
	Rendering::Rendering(App::App &rApp) : m_rApp{ rApp },
		window{ Window(DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT, APPLICATION_NAME) },
		m_device{ window },
		m_pipeline{
			m_device,
			(m_rApp.get_root() / "res/shaders/simple_shader.vert.spv").generic_string(),
			(m_rApp.get_root() / "res/shaders/simple_shader.frag.spv").generic_string(),
			Pipeline::default_pipeline_config_info(window.WIDTH, window.HEIGHT)
		}
	{
		ksc_log::debug("Rendering created.");
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