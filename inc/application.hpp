#pragma once

#include "mandelimage.hpp"
#include "nebulaimage.hpp"
#include <memory>

namespace foldscape
{
	class Application : public IDrawContext
	{
		GtkWidget* m_drawingArea;
		GtkWidget* m_menu;
		gulong m_tabSwitchId;
		std::unique_ptr<vk::Vulkan> m_vulkan;
		std::unique_ptr<ImageControlBase> m_imageControl;
	
	private:
		void Activate(GtkApplication* gtkApp);
		void ChangeTab(uint32_t page);
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
