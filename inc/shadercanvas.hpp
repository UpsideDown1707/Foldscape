#pragma once

#include <gtk/gtk.h>
#include <epoxy/gl.h>
#include <complex>

namespace foldscape
{
	using Cpx = std::complex<double>;

	struct Params
	{
		Cpx center;
		double xScale;
		double zoom;
		uint32_t maxIters;
	};

	struct ScreenLoc
	{
		double x;
		double y;
	};

	class IParamChangeListener
	{
	public:
		virtual void CenterChanged(Cpx center) = 0;
		virtual void ZoomChanged(double zoom) = 0;
		virtual void MaxItersChanged(uint32_t maxIters) = 0;
	};

	class ShaderCanvas
	{
		GtkWidget* m_glArea;

		GLuint m_vertexArray;
		GLuint m_vertexBuffer;
		GLuint m_program;
		GLuint m_centerLoc;
		GLuint m_xScaleLoc;
		GLuint m_zoomLoc;
		GLuint m_maxItersLoc;

		ScreenLoc m_cursor;
		ScreenLoc m_resolution;
		Cpx m_dragTimeCenter;
		Params m_params;

	private:
		void RealizeGl();
		void UnrealizeGl();
		gboolean Render();
		void Resize(ScreenLoc resolution);
		bool MakeGlCurrent();

		void DragBegin(ScreenLoc p);
		void DragUpdate(ScreenLoc p);
		void Zoom(double delta);

		Cpx ToCoords(ScreenLoc p) const;
		ScreenLoc ToScreen(Cpx p) const;

	public:
		ShaderCanvas();
		~ShaderCanvas();

		inline const Params& FractalParams() const { return m_params; }
		inline GtkWidget* Canvas() const { return m_glArea; }
	};
}