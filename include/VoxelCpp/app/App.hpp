#pragma once
#include <memory>
#include <VoxelCpp/rendering/Rendering.hpp>
#include <filesystem>

namespace App
{
	class App
	{
	public:
		App(const char *pNAME);
		~App();
		App(const App &) = delete;
		App &operator=(const App &) = delete;

		void loop() const;

		std::filesystem::path get_root();

		std::unique_ptr<Rendering::Rendering> m_Rendering;

		const char *get_name() const;
		const char *m_pAPP_NAME;

	private:
		void start_logging(const char *pAPP_NAME);

		std::filesystem::path m_root;
	};
}