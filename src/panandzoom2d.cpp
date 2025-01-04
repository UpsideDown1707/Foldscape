#include "panandzoom2d.hpp"
#include <iostream>

namespace foldscape
{
	std::complex<double> PanAndZoom2D::ToCoords(double2 p) const
	{
		return std::complex<double>{
			(p.x / (m_pzParams.resolution.x - 1.0) * 2.0 - 1.0) * (m_pzParams.resolution.x / m_pzParams.resolution.y * m_pzParams.zoom) + m_pzParams.center.real(),
			(p.y / (m_pzParams.resolution.y - 1.0) * -2.0 + 1.0) * m_pzParams.zoom + m_pzParams.center.imag()
		};
	}

	double2 PanAndZoom2D::ToScreen(std::complex<double> p) const
	{
		return double2{
			((p.real() - m_pzParams.center.real()) / (m_pzParams.resolution.x / m_pzParams.resolution.y * m_pzParams.zoom) + 1.0) * 0.5 * (m_pzParams.resolution.x - 1.0),
			((p.imag() - m_pzParams.center.imag()) / m_pzParams.zoom - 1.0) * -0.5 * (m_pzParams.resolution.y - 1.0)
		};
	}

	PanAndZoom2D::PanAndZoom2D(IDrawContext& drawContext)
		: m_drawContext{drawContext}
		, m_dragTimeCenter{}
		, m_pzParams{}
	{}

	void PanAndZoom2D::DragBegin(double2 p)
	{
		m_dragTimeCenter = m_pzParams.center;
	}

	void PanAndZoom2D::DragUpdate(double2 dp)
	{
		m_pzParams.center = m_dragTimeCenter - std::complex<double>{
			dp.x / (m_pzParams.resolution.x - 1.0) * 2.0 * m_pzParams.resolution.x / m_pzParams.resolution.y * m_pzParams.zoom,
			dp.y / (m_pzParams.resolution.y - 1.0) * -2.0 * m_pzParams.zoom};
		m_drawContext.RequestRender();
	}

	void PanAndZoom2D::Zoom(double delta)
	{
		const std::complex<double> zoomCenter = ToCoords(m_cursor);
		m_pzParams.zoom *= delta > 0 ? 1.25f : 0.8f;
		const double2 p = ToScreen(zoomCenter);
		m_pzParams.center = m_pzParams.center + std::complex<double>{
			(p.x - m_cursor.x) / (m_pzParams.resolution.x - 1.0) * 2.0 * m_pzParams.resolution.x / m_pzParams.resolution.y * m_pzParams.zoom,
			(p.y - m_cursor.y) / (m_pzParams.resolution.y - 1.0) * -2.0 * m_pzParams.zoom};
		m_drawContext.RequestRender();
	}
}