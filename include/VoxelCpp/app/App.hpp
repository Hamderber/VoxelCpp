#pragma once
#include <memory>
#include <filesystem>

namespace Rendering { class Rendering; };

namespace App
{
	class App
	{
	public:
		App();
		~App();

		App(const App &) = delete;
		App &operator=(const App &) = delete;

		void loop() const;

		std::filesystem::path get_root();

		std::unique_ptr<Rendering::Rendering> rendering;

	private:
		void start_logging();

		std::filesystem::path m_root;
	};
}