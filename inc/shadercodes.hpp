#pragma once

#include <gtk/gtk.h>
#include <epoxy/gl.h>

namespace foldscape
{
	namespace ShaderCode
	{
		extern const char CsCode[];
		extern const char VsCode[];
		extern const char FsCode[];
	}

	GLuint CreateShader(int type, const char* code);
}
