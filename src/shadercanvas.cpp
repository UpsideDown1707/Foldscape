#include "shadercanvas.hpp"
#include "shadercodes.hpp"
#include <iostream>

namespace foldscape
{
	static GLuint CreateShader(int type, const GLchar* code)
	{
		GLuint shader = glCreateShader(type);
		glShaderSource(shader, 1, &code, nullptr);
		glCompileShader(shader);

		int status;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
		if (!status)
		{
			int logLen;
			glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLen);
			char* log = new char[logLen + 1];
			glGetShaderInfoLog(shader, logLen, nullptr, log);
			log[logLen] = '\0';
			std::cerr << "Compile error: " << log << std::endl;
			delete[] log;
			glDeleteShader(shader);
			shader = 0;
		}
		return shader;
	}

	void ShaderCanvas::RealizeGl()
	{
		if (!MakeGlCurrent())
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

		m_centerLoc = glGetUniformLocation(m_program, "center");
		m_xScaleLoc = glGetUniformLocation(m_program, "xScale");
		m_zoomLoc = glGetUniformLocation(m_program, "zoom");
		m_maxItersLoc = glGetUniformLocation(m_program, "maxIters");

		glDetachShader(m_program, vs);
		glDetachShader(m_program, fs);

		glEnable(GL_CULL_FACE);
		glFrontFace(GL_CW);
		glCullFace(GL_BACK);
	}

	void ShaderCanvas::UnrealizeGl()
	{
		if (MakeGlCurrent())
		{
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

		glUseProgram(m_program);
		glBindVertexArray(m_vertexArray);
		glUniform2d(m_centerLoc, m_params.center.real(), m_params.center.imag());
		glUniform1d(m_xScaleLoc, m_params.xScale);
		glUniform1d(m_zoomLoc, m_params.zoom);
		glUniform1ui(m_maxItersLoc, m_params.maxIters);
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
		m_resolution = resolution;
		m_params.xScale = resolution.x / resolution.y;
	}

	bool ShaderCanvas::MakeGlCurrent()
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

	void ShaderCanvas::DragBegin(ScreenLoc p)
	{
		m_dragTimeCenter = m_params.center;
	}

	void ShaderCanvas::DragUpdate(ScreenLoc p)
	{
		m_params.center = m_dragTimeCenter + Cpx{
			p.x / (m_resolution.x - 1.0) * -2.0 * m_params.xScale * m_params.zoom,
			p.y / (m_resolution.y - 1.0) * 2.0 * m_params.zoom};
		gtk_gl_area_queue_render(GTK_GL_AREA(m_glArea));
	}

	void ShaderCanvas::Zoom(double delta)
	{
		const Cpx zoomCenter = ToCoords(m_cursor);
		m_params.zoom *= delta > 0 ? 1.25f : 0.8f;
		const ScreenLoc p = ToScreen(zoomCenter);
		m_params.center = m_params.center + Cpx{
			(p.x - m_cursor.x) / (m_resolution.x - 1.0) * 2.0 * m_params.xScale * m_params.zoom,
			(p.y - m_cursor.y) / (m_resolution.y - 1.0) * -2.0 * m_params.zoom};
		gtk_gl_area_queue_render(GTK_GL_AREA(m_glArea));
	}

	Cpx ShaderCanvas::ToCoords(ScreenLoc p) const
	{
		return Cpx{
			(p.x / (m_resolution.x - 1.0) * 2.0 - 1.0) * m_params.xScale * m_params.zoom + m_params.center.real(),
			(p.y / (m_resolution.y - 1.0) * 2.0 - 1.0) * m_params.zoom + m_params.center.imag()
		};
	}

	ScreenLoc ShaderCanvas::ToScreen(Cpx p) const
	{
		return ScreenLoc{
			((p.real() - m_params.center.real()) / (m_params.xScale * m_params.zoom) + 1.0) * 0.5 * (m_resolution.x - 1.0),
			((p.imag() - m_params.center.imag()) / m_params.zoom + 1.0) * 0.5 * (m_resolution.y - 1.0)
		};
	}

	ShaderCanvas::ShaderCanvas()
		: m_glArea{}
		, m_vertexArray{}
		, m_vertexBuffer{}
		, m_program{}
		, m_centerLoc{}
		, m_xScaleLoc{}
		, m_zoomLoc{}
		, m_cursor{}
		, m_resolution{}
		, m_dragTimeCenter{}
		, m_params{{-0.5, 0.0}, 1.0, 1.25, 256}
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
}