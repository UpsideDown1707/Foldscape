#include "imagecontrolbase.hpp"

namespace foldscape
{
	void ImageControlBase::CreateSurface(void* data, int width, int height)
	{
		SafeDestroySurface();
		m_surface = cairo_image_surface_create_for_data(reinterpret_cast<uint8_t*>(data), CAIRO_FORMAT_ARGB32, width, height, width * 4);
	}

	void ImageControlBase::SafeDestroySurface()
	 {
		if (m_surface)
		{
			cairo_surface_destroy(m_surface);
			m_surface = nullptr;
		}		
	 }

	ImageControlBase::ImageControlBase()
		: m_cursor{}
		, m_surface{nullptr}
	{}

	ImageControlBase::~ImageControlBase()
	{
		SafeDestroySurface();
	}

	void ImageControlBase::CursorMotion(double2 p)
	{
		m_cursor = p;
	}
}