#include "application.hpp"
#include <iostream>

namespace foldscape
{
	static constexpr int WIDTH = 1280;
	static constexpr int HEIGHT = 800;

	void Application::Activate(GtkApplication* gtkApp)
	{
		m_vulkan = std::make_unique<vk::Vulkan>("Foldscape", WIDTH, HEIGHT);
		m_mandelImage = std::make_unique<MandelImage>(*m_vulkan, *this, WIDTH, HEIGHT);
		m_surface = cairo_image_surface_create_for_data(reinterpret_cast<uint8_t*>(m_mandelImage->MappedImage()), CAIRO_FORMAT_ARGB32, WIDTH, HEIGHT, WIDTH * 4);

		m_drawingArea = gtk_drawing_area_new();
		
		g_signal_connect(GTK_DRAWING_AREA(m_drawingArea), "resize", G_CALLBACK(static_cast<void(*)(GtkDrawingArea*, gint, gint, Application*)>(
			[](GtkDrawingArea*, gint width, gint height, Application* app){
				app->Resize(width, height);
			}
		)), this);
		gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(m_drawingArea), static_cast<void(*)(GtkDrawingArea*, cairo_t*, int, int, gpointer)>(
			[](GtkDrawingArea*, cairo_t* cr, int w, int h, gpointer app){
				reinterpret_cast<Application*>(app)->Draw(cr, w, h);
		}), this, nullptr);

		GtkGesture* panDrag = gtk_gesture_drag_new();
		g_signal_connect(panDrag, "drag-begin", G_CALLBACK(static_cast<void(*)(GtkGestureDrag*, double, double, Application*)>([](GtkGestureDrag*, double x, double y, Application* app){
			app->DragBegin(double2{x, y});
		})), this);
		g_signal_connect(panDrag, "drag-update", G_CALLBACK(static_cast<void(*)(GtkGestureDrag*, double, double, Application*)>([](GtkGestureDrag* drag, double x, double y, Application* app){			
			app->DragUpdate(double2{x, y});
		})), this);
		gtk_widget_add_controller(m_drawingArea, GTK_EVENT_CONTROLLER(panDrag));

		GtkEventController* cursorWatcher = gtk_event_controller_motion_new();
		g_signal_connect(cursorWatcher, "motion", G_CALLBACK(static_cast<void(*)(GtkEventControllerMotion*, double, double, Application*)>([](GtkEventControllerMotion*, double x, double y, Application* app){
			app->CursorMotion(double2{x, y});
		})), this);
		gtk_widget_add_controller(m_drawingArea, cursorWatcher);

		GtkEventController* zoomController = gtk_event_controller_scroll_new(GTK_EVENT_CONTROLLER_SCROLL_VERTICAL);
		g_signal_connect(zoomController, "scroll", G_CALLBACK(static_cast<gboolean(*)(GtkEventControllerScroll*, double, double, Application*)>([](GtkEventControllerScroll*, double, double dy, Application* app){
			app->Zoom(dy);
			return static_cast<gboolean>(true);
		})), this);
		gtk_widget_add_controller(m_drawingArea, zoomController);

		GtkWidget* window = gtk_application_window_new(gtkApp);
		gtk_window_set_title(GTK_WINDOW(window), "Foldscape");
		gtk_window_set_default_size(GTK_WINDOW(window), WIDTH, HEIGHT);
		gtk_window_set_child(GTK_WINDOW(window), m_drawingArea);

		gtk_window_present(GTK_WINDOW(window));
	}

	void Application::Resize(int width, int height)
	{
		try
		{
			if (m_surface)
			{
				cairo_surface_destroy(m_surface);
				m_surface = nullptr;
			}
			m_mandelImage->Resize(width, height);
			m_surface = cairo_image_surface_create_for_data(reinterpret_cast<uint8_t*>(m_mandelImage->MappedImage()), CAIRO_FORMAT_ARGB32, width, height, width * 4);
		}
		catch (const std::exception& ex)
		{
			std::cout << ex.what() << std::endl;
		}
	}

	void Application::Draw(cairo_t* cr, int width, int height)
	{
		m_mandelImage->Render();
		cairo_set_source_surface(cr, m_surface, 0, 0);
		cairo_paint(cr);
	}

	void Application::DragBegin(double2 p)
	{
		m_mandelImage->DragBegin(p);
	}
	
	void Application::DragUpdate(double2 dp)
	{
		m_mandelImage->DragUpdate(dp);
	}

	void Application::CursorMotion(double2 p)
	{
		m_mandelImage->CursorMotion(p);
	}
	
	void Application::Zoom(double d)
	{
		m_mandelImage->Zoom(d);
	}

	Application::Application()
		: m_drawingArea{nullptr}
	{}

	Application::~Application()
	{
		if (m_surface)
		{
			cairo_surface_destroy(m_surface);
			m_surface = nullptr;
		}
		m_mandelImage.reset();
		m_vulkan.reset();
	}

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

	void Application::RequestRender()
	{
		gtk_widget_queue_draw(m_drawingArea);
	}
}
