#include "shaderimage.hpp"

namespace foldscape
{
	void ShaderImage::CreateTexture(int w, int h)
	{
		DeleteTexture();
		glGenTextures(1, &m_image);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_image);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, w, h, 0, GL_RGBA, GL_FLOAT, nullptr);
		glBindImageTexture(0, m_image, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
	}

	void ShaderImage::DeleteTexture()
	{
		if (m_image)
		{
			glDeleteTextures(1, &m_image);
			m_image = 0;
		}
	}

	ShaderImage::ShaderImage(IGlContext& context) 
		: m_context{context}
		, m_image{}
		, m_resolution{}
	{}

	ShaderImage::~ShaderImage()
	{
		DeleteTexture();
	}

	void ShaderImage::DragBegin(ScreenLoc p){}
	void ShaderImage::DragUpdate(ScreenLoc dp) {}
	void ShaderImage::Zoom(ScreenLoc cursor, double delta) {}

	void ShaderImage::Resize(ScreenLoc resolution)
	{
		m_resolution = resolution;
	}
}