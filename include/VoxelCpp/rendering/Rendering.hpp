#pragma once
#include <VoxelCpp/rendering/Pipeline.hpp>
#include <VoxelCpp/rendering/Device.hpp>
#include <VoxelCpp/rendering/Window.hpp>

namespace App { class App; }

namespace Rendering
{
	class Rendering
	{
	public:
		Rendering(App::App &rApp);
		~Rendering();

		Rendering(const Rendering &) = delete;
		Rendering &operator=(const Rendering &) = delete;

		void loop();

		Window window;

	private:
		App::App &m_rApp;
		Device m_device;
		Pipeline m_pipeline;
	};
}