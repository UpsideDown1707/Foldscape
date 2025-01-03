#pragma once

#include "mandelimage.hpp"
#include <gtk/gtk.h>
#include <memory>

namespace foldscape
{
	class Application
	{
		std::unique_ptr<vk::Vulkan> m_vulkan;
		std::unique_ptr<MandelImage> m_mandelImage;
	
	private:
		void Activate(GtkApplication* gtkApp);

		void Draw(cairo_t* cr, int width, int height);

	public:
		Application();
		~Application();

		int Main(int argc, char* argv[]);
	};
}
