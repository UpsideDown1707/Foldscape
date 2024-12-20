#include "application.hpp"

namespace foldscape
{
	void Application::Activate(GtkApplication* gtkApp)
	{
		const int width = 1000;
		const int height = 700;

		m_shaderCanvas = std::make_unique<ShaderCanvas>();

		GtkWidget* paned = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
		gtk_paned_set_end_child(GTK_PANED(paned), m_shaderCanvas->Canvas());
		gtk_paned_set_wide_handle(GTK_PANED(paned), true);
		gtk_paned_set_position(GTK_PANED(paned), width * 3 / 10);

		GtkWidget* window = gtk_application_window_new(gtkApp);
		gtk_window_set_title(GTK_WINDOW(window), "Foldscape");
		gtk_window_set_default_size(GTK_WINDOW(window), width, height);
		gtk_window_set_child(GTK_WINDOW(window), paned);

		gtk_window_present(GTK_WINDOW(window));
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
