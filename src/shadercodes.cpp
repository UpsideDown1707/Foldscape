#include "shadercodes.hpp"
#include <iostream>

namespace foldscape
{
	namespace ShaderCode
	{
		const char CsCode[] = 
R"(#version 460
precision highp float;
precision highp image2D;

layout(local_size_x = 8, local_size_y = 8) in;
layout(rgba32f, binding = 0) uniform image2D img_output;

uniform vec2 resolution;
uniform dvec2 center;
uniform double zoom;
uniform uint maxIters;

vec4 ToColor(float r)
{
	const vec3 colors[5] = vec3[](
			vec3(0, 7, 100) / 255.0,
			vec3(32, 107, 203) / 255.0,
			vec3(237, 255, 255) / 255.0,
			vec3(255, 170, 0) / 255.0,
			vec3(0, 2, 0) / 255.0);

	vec3 c1 = colors[int(mod(r, 5))];
	vec3 c2 = colors[int(mod(r + 1, 5))];
	return vec4(mix(c1, c2, mod(r, 1)), 1);
}

vec4 FractalColor(dvec2 coord)
{
	dvec2 z = dvec2(0, 0);
	dvec2 c = coord;
	for (uint i = 0; i < maxIters; ++i)
	{
		z = dvec2(z.x * z.x - z.y * z.y, 2 * z.x * z.y) + c;
		const float zLen = length(vec2(z.x, z.y));
		if (zLen > 4)
			return ToColor((i + 1 - log(log(zLen)) / log(2)) / 10);
	}
	return vec4(0, 0, 0, 1);
}

void main()
{
	vec2 uv = vec2(float(gl_GlobalInvocationID.x), float(gl_GlobalInvocationID.y)) / (resolution - 1) * 2 - 1;
	dvec2 coord = dvec2(uv.x * resolution.x / resolution.y, uv.y) * zoom + center;
	vec4 color = FractalColor(coord);
	imageStore(img_output, ivec2(gl_GlobalInvocationID.x, gl_GlobalInvocationID.y), color);
}
)";

		const char VsCode[] =
R"(#version 460
precision highp float;
layout (location = 0) in vec2 inPosition;
layout (location = 0) out vec2 fragTexcoord;

void main()
{
	fragTexcoord = inPosition * 0.5 + 0.5;
	gl_Position = vec4(inPosition, 1, 1);
}
)";

		const char FsCode[] =
R"(#version 460
precision highp float;
layout (location = 0) in vec2 fragTexcoord;
layout (location = 0) out vec4 outColor;

uniform sampler2D inTexture;

void main()
{
	outColor = vec4(texture(inTexture, fragTexcoord).xyz, 1);
}
)";
	}

	GLuint CreateShader(int type, const char* code)
	{
		GLuint shader = glCreateShader(type);
		glShaderSource(shader, 1, &code, nullptr);
		glCompileShader(shader);

		int status;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
		if (!status)
		{
			int logLen;
			glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLen);
			char* log = new char[logLen + 1];
			glGetShaderInfoLog(shader, logLen, nullptr, log);
			log[logLen] = '\0';
			std::cerr << "Compile error: " << log << std::endl;
			delete[] log;
			glDeleteShader(shader);
			shader = 0;
		}
		return shader;
	}
}
