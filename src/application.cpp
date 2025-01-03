#include "application.hpp"
#include <iostream>

namespace foldscape
{
	static constexpr int WIDTH = 1280;
	static constexpr int HEIGHT = 800;

	void Application::Activate(GtkApplication* gtkApp)
	{
		m_vulkan = std::make_unique<vk::Vulkan>("Foldscape", WIDTH, HEIGHT);

		GtkWidget* drawingArea = gtk_drawing_area_new();
		
		g_signal_connect(GTK_DRAWING_AREA(drawingArea), "resize", G_CALLBACK(static_cast<void(*)(GtkDrawingArea*, gint, gint, Application*)>(
			[](GtkDrawingArea*, gint width, gint height, Application* app){
				try
				{
					app->m_vulkan->Resize(width, height);
				}
				catch (const std::exception& ex)
				{
					std::cout << ex.what() << std::endl;
				}
			}
		)), this);
		gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(drawingArea), static_cast<void(*)(GtkDrawingArea*, cairo_t*, int, int, gpointer)>(
			[](GtkDrawingArea*, cairo_t* cr, int w, int h, gpointer app){
				reinterpret_cast<Application*>(app)->Draw(cr, w, h);
		}), this, nullptr);

		GtkWidget* window = gtk_application_window_new(gtkApp);
		gtk_window_set_title(GTK_WINDOW(window), "Foldscape");
		gtk_window_set_default_size(GTK_WINDOW(window), WIDTH, HEIGHT);
		gtk_window_set_child(GTK_WINDOW(window), drawingArea);

		gtk_window_present(GTK_WINDOW(window));
	}

	void Application::Draw(cairo_t* cr, int width, int height)
	{
		m_vulkan->Render();

		cairo_surface_t* surface = cairo_image_surface_create_for_data(reinterpret_cast<uint8_t*>(m_vulkan->MappedImage()), CAIRO_FORMAT_ARGB32, width, height, width * 4);

		cairo_set_source_surface(cr, surface, 0, 0);
		cairo_paint(cr);

		cairo_surface_destroy(surface);
	}

	Application::Application()
	{}

	Application::~Application()
	{}

	int Application::Main(int argc, char* argv[])
	{
		GtkApplication* gtkApp = gtk_application_new("app.praggmars.foldscape", G_APPLICATION_DEFAULT_FLAGS);
		g_signal_connect(gtkApp, "activate", G_CALLBACK(static_cast<void(*)(GtkApplication*, Application*)>([](GtkApplication* gtkApp, Application* app){
			app->Activate(gtkApp);
		})), this);
		const int status = g_application_run(G_APPLICATION(gtkApp), argc, argv);
		g_object_unref(gtkApp);
		return status;
	}
}
