#pragma once

#include "shaderimage.hpp"

namespace foldscape
{
	class IGlCallbacks
	{
	public:
		virtual void Realize() = 0;
		virtual void Unrealize() = 0;
	};

	class ShaderCanvas : public IGlContext
	{
		GtkWidget* m_glArea;
		ShaderImage* m_activeImage;
		IGlCallbacks* m_callbacks;

		GLuint m_vertexArray;
		GLuint m_vertexBuffer;
		GLuint m_program;

		ScreenLoc m_cursor;

	private:
		void RealizeGl();
		void UnrealizeGl();
		gboolean Render();
		void Resize(ScreenLoc resolution);

		void DragBegin(ScreenLoc p);
		void DragUpdate(ScreenLoc p);
		void Zoom(double delta);

	public:
		explicit ShaderCanvas(IGlCallbacks* callbacks = nullptr);
		~ShaderCanvas();

		inline void ActiveImage(ShaderImage* image) { m_activeImage = image; }

		virtual bool MakeCurrent() override;
		virtual void RequestRender() override;
		inline GtkWidget* Canvas() const { return m_glArea; }
	};
}