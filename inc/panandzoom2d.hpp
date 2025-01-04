#pragma once

#include "imagecontrolbase.hpp"
#include <complex>

namespace foldscape
{
	struct alignas(16) PanAndZoomParams
	{
		std::complex<double> center;
		double zoom;
		int2 resolution;
	};

	class IDrawContext
	{
	public:
		virtual void RequestRender() = 0;
	};

	class PanAndZoom2D : public ImageControlBase
	{
		IDrawContext& m_drawContext;
		std::complex<double> m_dragTimeCenter;
	
	protected:
		PanAndZoomParams m_pzParams;
	
	protected:
		std::complex<double> ToCoords(double2 p) const;
		double2 ToScreen(std::complex<double> p) const;

	public:
		explicit PanAndZoom2D(IDrawContext& drawContext);
		virtual void DragBegin(double2 p) override;
		virtual void DragUpdate(double2 dp) override;
		virtual void Scroll(double delta) override;
	};
}