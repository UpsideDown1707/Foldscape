#pragma once

#include <gtk/gtk.h>

namespace foldscape
{
	struct double2
	{
		double x;
		double y;
	};

	struct int2
	{
		int x;
		int y;
	};

	class ImageControlBase
	{
	protected:
		double2 m_cursor;
		cairo_surface_t* m_surface;

	protected:
		void CreateSurface(void* data, int width, int height);
		void SafeDestroySurface();

	public:
		ImageControlBase();
		virtual ~ImageControlBase();

		virtual void DragBegin(double2 p) = 0;
		virtual void DragUpdate(double2 dp) = 0;
		virtual void Scroll(double delta) = 0;
		void CursorMotion(double2 p);

		virtual void Resize(int width, int height) = 0;
		virtual void Render() = 0;
		inline cairo_surface_t* Surface() const { return m_surface; }
	};
}