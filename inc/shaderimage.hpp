#pragma once

#include "shadercodes.hpp"
#include <complex>

namespace foldscape
{
	using Cpx = std::complex<double>;

	struct ScreenLoc
	{
		double x;
		double y;
	};

	class IGlContext
	{
	public:
		virtual bool MakeCurrent() = 0;
		virtual void RequestRender() = 0;
	};

	class ShaderImage
	{
	protected:
		IGlContext& m_context;
		GLuint m_image;
		ScreenLoc m_resolution;

	protected:
		void CreateTexture(int w, int h);
		void DeleteTexture();

	public:
		ShaderImage(IGlContext& context);
		virtual ~ShaderImage();
		virtual void DragBegin(ScreenLoc p);
		virtual void DragUpdate(ScreenLoc dp);
		virtual void Zoom(ScreenLoc cursor, double delta);
		virtual void Resize(ScreenLoc resolution);
		virtual void Render() = 0;
		inline GLuint Image() const { return m_image; }
	};
}