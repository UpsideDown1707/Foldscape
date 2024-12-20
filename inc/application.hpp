#pragma once

#include "shadercanvas.hpp"
#include <memory>

namespace foldscape
{
	class Application
	{
		std::unique_ptr<ShaderCanvas> m_shaderCanvas;

	private:
		void Activate(GtkApplication* gtkApp);

	public:
		Application();
		~Application();

		int Main(int argc, char* argv[]);
	};
}
