#include "mandelimage.hpp"
#include <iostream>

namespace foldscape
{
	Cpx MandelImage::ToCoords(ScreenLoc p) const
	{
		return Cpx{
			(p.x / (m_resolution.x - 1.0) * 2.0 - 1.0) * m_resolution.x / m_resolution.y * m_params.zoom + m_params.center.real(),
			(p.y / (m_resolution.y - 1.0) * 2.0 - 1.0) * m_params.zoom + m_params.center.imag()
		};
	}

	ScreenLoc MandelImage::ToScreen(Cpx p) const
	{
		return ScreenLoc{
			((p.real() - m_params.center.real()) / (m_resolution.x / m_resolution.y * m_params.zoom) + 1.0) * 0.5 * (m_resolution.x - 1.0),
			((p.imag() - m_params.center.imag()) / m_params.zoom + 1.0) * 0.5 * (m_resolution.y - 1.0)
		};
	}

	MandelImage::MandelImage(IGlContext& context)
		: ShaderImage(context)
		, m_program{}
		, m_resolutionLoc{}
		, m_centerLoc{}
		, m_zoomLoc{}
		, m_maxItersLoc{}
		, m_dragTimeCenter{}
		, m_params{{-0.5, 0.0}, 1.25, 256}
	{
		GLuint fracShader = CreateShader(GL_COMPUTE_SHADER, ShaderCode::CsCode);
		if (!fracShader)
			return;
		m_program = glCreateProgram();
		glAttachShader(m_program, fracShader);
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
			glDeleteShader(fracShader);
			return;
		}

		m_resolutionLoc = glGetUniformLocation(m_program, "resolution");
		m_centerLoc = glGetUniformLocation(m_program, "center");
		m_zoomLoc = glGetUniformLocation(m_program, "zoom");
		m_maxItersLoc = glGetUniformLocation(m_program, "maxIters");

		glDetachShader(m_program, fracShader);
	}

	MandelImage::~MandelImage()
	{
		if (m_program)
		{
			glDeleteProgram(m_program);
			m_program = 0;
		}
	}

	void MandelImage::DragBegin(ScreenLoc p)
	{
		m_dragTimeCenter = m_params.center;
	}
	
	void MandelImage::DragUpdate(ScreenLoc dp)
	{
		m_params.center = m_dragTimeCenter + Cpx{
			dp.x / (m_resolution.x - 1.0) * -2.0 * m_resolution.x / m_resolution.y * m_params.zoom,
			dp.y / (m_resolution.y - 1.0) * 2.0 * m_params.zoom};
		m_context.RequestRender();
	}
	
	void MandelImage::Zoom(ScreenLoc cursor, double delta)
	{
		const Cpx zoomCenter = ToCoords(cursor);
		m_params.zoom *= delta > 0 ? 1.25f : 0.8f;
		const ScreenLoc p = ToScreen(zoomCenter);
		m_params.center = m_params.center + Cpx{
			(p.x - cursor.x) / (m_resolution.x - 1.0) * 2.0 * m_resolution.x / m_resolution.y * m_params.zoom,
			(p.y - cursor.y) / (m_resolution.y - 1.0) * -2.0 * m_params.zoom};
		m_context.RequestRender();
	}
	
	void MandelImage::Resize(ScreenLoc resolution)
	{
		ShaderImage::Resize(resolution);
		CreateTexture(int(resolution.x), int(resolution.y));
	}
	
	void MandelImage::Render()
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_image);
		glUseProgram(m_program);

		glUniform2f(m_resolutionLoc, m_resolution.x, m_resolution.y);
		glUniform2d(m_centerLoc, m_params.center.real(), m_params.center.imag());
		glUniform1d(m_zoomLoc, m_params.zoom);
		glUniform1ui(m_maxItersLoc, m_params.maxIters);

		glDispatchCompute((int(m_resolution.x )+ 7) / 8, (int(m_resolution.y) + 7) / 8, 1);
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	}
}