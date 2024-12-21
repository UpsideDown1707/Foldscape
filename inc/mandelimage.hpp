#pragma once

#include "shaderimage.hpp"

namespace foldscape
{
	struct Params
	{
		Cpx center;
		double zoom;
		uint32_t maxIters;
	};

	class MandelImage : public ShaderImage
	{
		GLuint m_program;
		GLuint m_resolutionLoc;
		GLuint m_centerLoc;
		GLuint m_zoomLoc;
		GLuint m_maxItersLoc;

		Cpx m_dragTimeCenter;
		Params m_params;

	private:
		Cpx ToCoords(ScreenLoc p) const;
		ScreenLoc ToScreen(Cpx p) const;

	public:
		explicit MandelImage(IGlContext& context);
		virtual ~MandelImage();
		virtual void DragBegin(ScreenLoc p) override;
		virtual void DragUpdate(ScreenLoc dp) override;
		virtual void Zoom(ScreenLoc cursor, double delta) override;
		virtual void Resize(ScreenLoc resolution) override;
		virtual void Render() override;

		inline const Params& FractalParams() const { return m_params; }
	};
}