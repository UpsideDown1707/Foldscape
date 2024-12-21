#pragma once

#include "mandelimage.hpp"
#include "shadercanvas.hpp"
#include <memory>

namespace foldscape
{
	class Application : private IGlCallbacks
	{
		std::unique_ptr<ShaderCanvas> m_shaderCanvas;
		std::unique_ptr<ShaderImage> m_shaderImage;

	private:
		void Activate(GtkApplication* gtkApp);
		virtual void Realize() override;
		virtual void Unrealize() override;

	public:
		Application();
		~Application();

		int Main(int argc, char* argv[]);
	};
}
