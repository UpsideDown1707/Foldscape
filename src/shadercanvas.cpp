#include "shadercanvas.hpp"
#include <iostream>

namespace foldscape
{	
	void ShaderCanvas::RealizeGl()
	{
		if (!MakeCurrent())
			return;

		const float vertices[] = {
			-1.0f, 1.0f,
			1.0f, 1.0f,
			1.0f, -1.0f,
			1.0f, -1.0f,
			-1.0f, -1.0f,
			-1.0f, 1.0f
		};

		glGenVertexArrays(1, &m_vertexArray);
		glBindVertexArray(m_vertexArray);
		glGenBuffers(1, &m_vertexBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, nullptr);
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		GLuint vs = CreateShader(GL_VERTEX_SHADER, ShaderCode::VsCode);
		if (!vs)
			return;
		GLuint fs = CreateShader(GL_FRAGMENT_SHADER, ShaderCode::FsCode);
		if (!fs)
		{
			glDeleteShader(vs);
			return ;
		}
		m_program = glCreateProgram();
		glAttachShader(m_program, vs);
		glAttachShader(m_program, fs);
		glLinkProgram(m_program);

		int status;
		glGetProgramiv(m_program, GL_LINK_STATUS, &status);
		if (!status)
		{
			int logLen;
			glGetProgramiv(m_program, GL_INFO_LOG_LENGTH, &logLen);
			char* log = new char[logLen + 1];
			glGetProgramInfoLog(m_program, logLen, nullptr, log);
			log[logLen] = '\0';
			std::cerr << "Link error: " << log << std::endl;
			delete[] log;
			m_program = 0;
			glDeleteShader(vs);
			glDeleteShader(fs);
			return;
		}

		glDetachShader(m_program, vs);
		glDetachShader(m_program, fs);

		glEnable(GL_CULL_FACE);
		glFrontFace(GL_CW);
		glCullFace(GL_BACK);

		if (m_callbacks)
			m_callbacks->Realize();
	}

	void ShaderCanvas::UnrealizeGl()
	{
		if (MakeCurrent())
		{
			if (m_callbacks)
				m_callbacks->Unrealize();
			glDeleteBuffers(1, &m_vertexBuffer);
			glDeleteProgram(m_program);
		}
	}

	gboolean ShaderCanvas::Render()
	{
		GtkGLArea* gl = GTK_GL_AREA(m_glArea);
		if (GError* err = gtk_gl_area_get_error(gl))
		{
			std::cerr << "OpenGL error: " << err->message << std::endl;
			return FALSE;
		}

		if (m_activeImage)
			m_activeImage->Render();

		glUseProgram(m_program);
		if (m_activeImage)
		{
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, m_activeImage->Image());
		}
		glBindVertexArray(m_vertexArray);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		glBindVertexArray(0);
		glDisableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glUseProgram(0);

		glFlush();
		gtk_gl_area_queue_render(gl);
		return TRUE;
	}

	void ShaderCanvas::Resize(ScreenLoc resolution)
	{
		if (m_activeImage)
			m_activeImage->Resize(resolution);
	}

	void ShaderCanvas::DragBegin(ScreenLoc p)
	{
		if (m_activeImage)
			m_activeImage->DragBegin(p);
	}

	void ShaderCanvas::DragUpdate(ScreenLoc p)
	{
		if (m_activeImage)
			m_activeImage->DragUpdate(p);
	}

	void ShaderCanvas::Zoom(double delta)
	{
		if (m_activeImage)
			m_activeImage->Zoom(m_cursor, delta);
	}

	ShaderCanvas::ShaderCanvas(IGlCallbacks* callbacks)
		: m_glArea{}
		, m_activeImage{}
		, m_callbacks{callbacks}
		, m_vertexArray{}
		, m_vertexBuffer{}
		, m_program{}
		, m_cursor{}
	{
		m_glArea = gtk_gl_area_new();
		gtk_gl_area_set_allowed_apis(GTK_GL_AREA(m_glArea), GDK_GL_API_GL);
		gtk_gl_area_set_required_version(GTK_GL_AREA(m_glArea), 4, 6);
		if (GError* err = gtk_gl_area_get_error(GTK_GL_AREA(m_glArea)))
			std::cerr << err->message << std::endl;
		g_signal_connect(m_glArea, "realize", G_CALLBACK(static_cast<void(*)(GtkWidget*, ShaderCanvas*)>([](GtkWidget* widget, ShaderCanvas* app){
			app->RealizeGl();
		})), this);
		g_signal_connect(m_glArea, "unrealize", G_CALLBACK(static_cast<void(*)(GtkWidget*, ShaderCanvas*)>([](GtkWidget* widget, ShaderCanvas* app){
			app->UnrealizeGl();
		})), this);
		g_signal_connect(m_glArea, "render", G_CALLBACK(static_cast<gboolean(*)(GtkGLArea*, GdkGLContext*, ShaderCanvas*)>([](GtkGLArea* gl, GdkGLContext* context, ShaderCanvas* app){
			return app->Render();
		})), this);
		g_signal_connect(m_glArea, "resize", G_CALLBACK(static_cast<void(*)(GtkGLArea*, int, int, ShaderCanvas*)>([](GtkGLArea*, int width, int height, ShaderCanvas* app){
			app->Resize(ScreenLoc{static_cast<double>(width), static_cast<double>(height)});
		})), this);

		GtkGesture* panDrag = gtk_gesture_drag_new();
		g_signal_connect(panDrag, "drag-begin", G_CALLBACK(static_cast<void(*)(GtkGestureDrag*, double, double, ShaderCanvas*)>([](GtkGestureDrag*, double x, double y, ShaderCanvas* app){
			app->DragBegin(ScreenLoc{x, y});
		})), this);
		g_signal_connect(panDrag, "drag-update", G_CALLBACK(static_cast<void(*)(GtkGestureDrag*, double, double, ShaderCanvas*)>([](GtkGestureDrag* drag, double x, double y, ShaderCanvas* app){			
			app->DragUpdate(ScreenLoc{x, y});
		})), this);
		gtk_widget_add_controller(m_glArea, GTK_EVENT_CONTROLLER(panDrag));

		GtkEventController* cursorWatcher = gtk_event_controller_motion_new();
		g_signal_connect(cursorWatcher, "motion", G_CALLBACK(static_cast<void(*)(GtkEventControllerMotion*, double, double, ShaderCanvas*)>([](GtkEventControllerMotion*, double x, double y, ShaderCanvas* app){
			app->m_cursor = ScreenLoc{x, y};
		})), this);
		gtk_widget_add_controller(m_glArea, cursorWatcher);

		GtkEventController* zoomController = gtk_event_controller_scroll_new(GTK_EVENT_CONTROLLER_SCROLL_VERTICAL);
		g_signal_connect(zoomController, "scroll", G_CALLBACK(static_cast<gboolean(*)(GtkEventControllerScroll*, double, double, ShaderCanvas*)>([](GtkEventControllerScroll*, double, double dy, ShaderCanvas* app){
			app->Zoom(dy);
			return static_cast<gboolean>(true);
		})), this);
		gtk_widget_add_controller(m_glArea, zoomController);
	}

	ShaderCanvas::~ShaderCanvas()
	{}

	bool ShaderCanvas::MakeCurrent()
	{
		GtkGLArea* gl = GTK_GL_AREA(m_glArea);
		gtk_gl_area_make_current(gl);
		if (GError* err = gtk_gl_area_get_error(gl))
		{
			std::cerr << "OpenGL error: " << err->message << std::endl;
			return false;
		}
		return true;
	}

	void ShaderCanvas::RequestRender()
	{
		gtk_gl_area_queue_render(GTK_GL_AREA(m_glArea));
	}
}