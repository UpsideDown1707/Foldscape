#include "shadercodes.hpp"

namespace foldscape::ShaderCode
{
	const char VsCode[] =
R"(#version 460
precision highp float;
layout (location = 0) in vec2 inPosition;
layout (location = 0) out vec2 fragTexcoord;

void main()
{
	fragTexcoord = inPosition;
	gl_Position = vec4(inPosition, 1, 1);
}
)";

	const char FsCode[] =
R"(#version 460
precision highp float;
layout (location = 0) in vec2 fragTexcoord;
layout (location = 0) out vec4 outColor;

uniform dvec2 center;
uniform double xScale;
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
	dvec2 coord = dvec2(fragTexcoord.x * xScale, fragTexcoord.y) * zoom + center;
	outColor = FractalColor(coord);
}
)";
}
