#pragma once
#include <VoxelCpp/rendering/Window.hpp>
#include <memory>
#include <VoxelCpp/rendering/Pipeline.hpp>
#include <format>

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

		App::App &m_rApp;
		std::unique_ptr<Window> m_Window;

	private:
		Pipeline m_Pipeline;
	};
}