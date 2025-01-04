#pragma once

#include "mandelimage.hpp"
#include <gtk/gtk.h>
#include <memory>

namespace foldscape
{
	class Application : public IDrawContext
	{
		GtkWidget* m_drawingArea;
		cairo_surface_t* m_surface;
		std::unique_ptr<vk::Vulkan> m_vulkan;
		std::unique_ptr<MandelImage> m_mandelImage;
	
	private:
		void Activate(GtkApplication* gtkApp);
		void Resize(int width, int height);
		void Draw(cairo_t* cr, int width, int height);
		void DragBegin(double2 p);
		void DragUpdate(double2 dp);
		void CursorMotion(double2 p);
		void Zoom(double d);
	
	public:
		Application();
		~Application();

		int Main(int argc, char* argv[]);
		virtual void RequestRender() override;
	};
}
